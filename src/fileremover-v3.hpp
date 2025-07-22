#pragma once
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Milivoj (Mike) DAVIDOV
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
/////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "get_readable_thread_id.hpp"
#include "set_thread_name.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <iostream>
#include <string>
#include <QDebug>

namespace fs = std::filesystem;
class QString;

namespace Frv3
{
using ProgressCallback = std::function<void(int, const QString&, uint64_t, bool, uint64_t)>;
using CompletionCallback = std::function<void(bool)>;

using rec_dir_it = fs::recursive_directory_iterator;
using dir_opts = fs::directory_options;

using std::cout;
using std::endl;


class FileRemover
{
public:
    FileRemover() {
        //qDebug() << "Frv3::FileRemover CTOR";
        stop_req = false;
    }

    ~FileRemover() {
        //qDebug() << "Frv3::FileRemover DTOR";
    }

    void removeFiles(const IntQStringMap& rowPathMap)
    {
        worker_ = std::jthread([this, rowPathMap](const std::stop_token&) {
            stop_req = false;
            rmFilesAndDirs(rowPathMap);
        });
    }

    // Set callback for progress updates
    void setProgressCallback(ProgressCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        progressCallback_ = std::move(callback);
    }

    // Set callback for progress updates
    void setCompletionCallback(CompletionCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        completionCallback_ = std::move(callback);
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_req = true;
        worker_.request_stop();
    }

    struct DeepDirCount {
        uint64_t files = 0;
        uint64_t folders = 0;
    };

    DeepDirCount deepCount(const fs::path& path) {
        DeepDirCount count;
        for (const auto& entry : rec_dir_it(path, dir_opts::skip_permission_denied)) {
            if (stop_req)
                return count;
            if (entry.is_directory())
                count.folders++;
            else
                count.files++;
        }
        return count;
    }

    bool deepRemoveFiles(int row, const fs::path& path, uint64_t& nbrDel, uint64_t& size) {
        auto rmOk = true;
        try {
            if (!fs::is_directory(path)) {
                const auto sz = fs::file_size(path);  // do it BEFORE removal
                if (fs::remove(path)) {
                    nbrDel++;
                    size += sz;
                }
                else
                    rmOk = false;
                std::lock_guard<std::mutex> lock(mutex_);
                if (progressCallback_) {
                    progressCallback_(row, QString::fromStdString(path.string()), size, rmOk, nbrDel);
                    // qDebug() << "rmOk:" << rmOk << "removed file:" << path.string().c_str() << "nbrDel:" << nbrDel;
                }
                return rmOk;
            }
            for (const auto& entry : rec_dir_it(path, dir_opts::skip_permission_denied)) {
                if (stop_req)
                    return rmOk;
                try {
                    if (!entry.is_directory()) {
                        const auto sz = entry.file_size();  // do it BEFORE removal
                        if (fs::remove(entry.path())) {
                            nbrDel++;
                            size += sz;
                        }
                        else
                            rmOk = false;
                        std::lock_guard<std::mutex> lock(mutex_);
                        if (progressCallback_) {
                            progressCallback_(row, QString::fromStdString(entry.path().string()), size, rmOk, nbrDel);
                            // qDebug() << "rmOk:" << rmOk << "removed file:" << entry.path().string().c_str() << "nbrDel:" << nbrDel;
                        }
                    }
                }
                catch (const fs::filesystem_error& ex) {
                    rmOk = false;
                    cout << "[EXCEPTION] " << ex.what() << endl;
                }
            }
        }
        catch (const fs::filesystem_error& ex) {
            rmOk = false;
            cout << "ERROR with " << path << " | " << ex.what() << endl;
        }
        return rmOk;
    }

private:
    void rmFilesAndDirs(const IntQStringMap& rowPathMap)
    {
        set_thread_name("Frv3FileRemover");
        const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        qDebug() << "Frv3: rmFilesAndDirs: my thread id:" << get_readable_thread_id() << " hash:" << tid;
        auto success = true;
        auto nbrDel = uint64_t(0);
        auto size = (uint64_t)0;
        for (const auto& [row, pathQstr] : rowPathMap) {
            if (stop_req)
                break;
            const auto path = pathQstr.toStdString();
            auto rmOk = false;
            try {
                // Remove all files in all subdirs
                if (!deepRemoveFiles(row, path, nbrDel, size)) {
                    success = false;
                }
                if (stop_req)
                    break;
                // Dirs are now empty, it's going to be fast to remove all
                if (fs::is_directory(path)) {
                    const auto nd = fs::remove_all(path);
                    rmOk = (nd > 0);
                    nbrDel += nd;
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (progressCallback_) {
                        progressCallback_(row, QString::fromStdString(path), size, rmOk, nbrDel);
                        qDebug() << "rmOk:" << rmOk << "removed dir:" << path.c_str() << "nd:" << nd << "nbrDel:" << nbrDel;
                    }
                    if (nd <= 0)
                        success = false;
                }
            }
            catch (const fs::filesystem_error& e) {
                success = false;
                qDebug() << "Remove ERROR:" << e.what();
            }
        }
        if (completionCallback_) {
            completionCallback_(success);
        }
        qDebug() << "Frv3: thread FUNCTION finished, NBR DELETED:" << nbrDel;
    }

    std::jthread worker_;
    std::mutex mutex_;
    bool stop_req{ false };
    ProgressCallback progressCallback_;
    CompletionCallback completionCallback_;
};
}
