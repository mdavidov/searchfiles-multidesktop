#pragma once
//
// Copyright (c) Milivoj (Mike) DAVIDOV
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//

#include "ifileremover.hpp"
#include "common.hpp"
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
using rec_dir_it = fs::recursive_directory_iterator;
using dir_opts = fs::directory_options;

using std::cout;
using std::endl;

/// @brief FileRemover v3 class for removing files and folders.
/// It uses a std::jthread (C++20) to perform the removal in a separate thread.
/// It provides a callback mechanism for progress updates and completion.
/// The progress callback is called for each file/folder removed,
/// and the completion callback is called when the removal is complete.
/// It uses std::filesystem and std::filesystem::recursive_directory_iterator
/// for file and directory operations.
/// @author Milivoj (Mike) DAVIDOV
///
class FileRemover : public mmd::IFileRemover
{
public:
    explicit FileRemover(QObject* uiObject) : m_uiObject(uiObject) {
        stop_req = false;
    }

    ~FileRemover() override {
    }

    void removeFilesAndFolders(
        const IntQStringMap& rowPathMap,
        mmd::ProgressCallback progressCb,
        mmd::CompletionCallback completionCb
    ) override
    {
        // Store callbacks as member variables or use shared_ptr to extend lifetime
        progressCallback_ = std::move(progressCb);
        completionCallback_ = std::move(completionCb);

        // Create jthread with captures by 'this' to class members
        worker_ = std::jthread([this, rowPathMap](std::stop_token stoken) {
                stop_token_ = stoken;
                stop_req = false;
                rmFilesAndDirs(this, rowPathMap);
            }
        );
        worker_.detach(); // Detach the thread to allow it to run independently
    }

    void stop() override {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_req = true;
        worker_.request_stop();
    }

    /// Set progress updates callback
    void setProgressCallback(mmd::ProgressCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        progressCallback_ = std::move(callback);
    }

    /// Set completion callback
    void setCompletionCallback(mmd::CompletionCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        completionCallback_ = std::move(callback);
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

    bool removeFile(int row, const fs::path& path, uint64_t& nbrDel, uint64_t& size) {
        auto rmOk = true;
        std::error_code ec;
        const auto sz = fs::file_size(path);  // do it BEFORE removal
        if (fs::remove(path, ec)) {
            nbrDel++;
            size += sz;
        }
        else {
            rmOk = false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (progressCallback_) {
            progressCallback_(row, QString::fromStdString(path.string()), size, rmOk, nbrDel);
        }
        return rmOk;
    }

    bool removeFilesInSubfolders(int row, const fs::path& path, uint64_t& nbrDel, uint64_t& size) {
        auto rmOk = true;
        std::error_code ec;
        for (const auto& entry : rec_dir_it(path, dir_opts::skip_permission_denied)) {
            if (stop_req)
                return rmOk;
            try {
                if (!entry.exists(ec)) {
                    continue; // It could have been already removed
                }
                if (!entry.is_directory(ec)) {
                    rmOk = removeFile(row, entry.path(), nbrDel, size);
                }
            }
            catch (const fs::filesystem_error& ex) {
                rmOk = false;
                cout << "EXCEPTION: " << ex.what() << endl;
            }
        }
        return rmOk;
    }

    bool deepRemoveFiles(int row, const fs::path& path, uint64_t& nbrDel, uint64_t& size) {
        auto rmOk = true;
        try {
            std::error_code ec;
            if (!fs::exists(path, ec)) {
                return true; // It could have been already removed
            }
            if (!fs::is_directory(path, ec)) {
                rmOk = removeFile(row, path, nbrDel, size);
            }
            else {
                rmOk = removeFilesInSubfolders(row, path, nbrDel, size);
            }
        }
        catch (const fs::filesystem_error& ex) {
            rmOk = false;
            cout << "EXCEPTION: " << ex.what() << endl;
        }
        return rmOk;
    }

private:
    void rmFilesAndDirs(FileRemover* ptr, const IntQStringMap& rowPathMap)
    {
        set_thread_name("Frv3FileRemover");
        const auto handle = ptr->worker_.native_handle(); (void)handle;
        // qDebug() << "Frv3::FileRemover: Thread STARTED;  name: Frv3FileRemover" << "native_handle:" << handle;
        auto success = true;
        auto nbrDel = uint64_t(0);
        auto size = (uint64_t)0;
        for (const auto& [row, pathQstr] : rowPathMap) {
            if (stop_req)
                return;
            const auto path = pathQstr.toStdString();
            auto rmOk = false;
            std::error_code ec;
            if (!fs::exists(path, ec)) {
                continue; // It could have been already removed
            }
            try {
                // Remove all files in all subdirs
                if (!deepRemoveFiles(row, path, nbrDel, size)) {
                    success = false;
                }
                if (stop_req)
                    return;
                // Dirs are now empty, it's going to be fast to remove all
                if (fs::is_directory(path, ec)) {
                    const auto nd = fs::remove_all(path, ec);
                    rmOk = (ec.value() == 0);
                    nbrDel += nd;
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (progressCallback_) {
                        QMetaObject::invokeMethod(m_uiObject,
                            [this, row, pathQstr, size, rmOk, nbrDel]() { progressCallback_(row, pathQstr, size, rmOk, nbrDel); },
                            Qt::QueuedConnection);
                    }
                    if (!rmOk)
                        success = false;
                }
            }
            catch (const fs::filesystem_error& e) {
                success = false;
                qDebug() << "EXCEPTION:" << e.what();
            }
        }
        if (completionCallback_) {
            QMetaObject::invokeMethod(m_uiObject,
                [this, success]() { completionCallback_(success); },
                Qt::QueuedConnection);
        }
    }

    std::jthread worker_;
    std::mutex mutex_;
    std::stop_token stop_token_;
    bool stop_req{ false };
    QObject* m_uiObject;
    mmd::ProgressCallback progressCallback_;
    mmd::CompletionCallback completionCallback_;
};
}
