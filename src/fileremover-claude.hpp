#pragma once

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

namespace fs = std::filesystem;
class QString;

namespace Claude
{
using ProgressCallback = std::function<void(int, const QString&, uint64_t, bool, uint64_t)>;
using CompletionCallback = std::function<void(bool)>;

using rec_dir_it = fs::recursive_directory_iterator;
using dir_opts = fs::directory_options;

class FileRemover
{
public:
    FileRemover() {
        qDebug() << "Claude::FileRemover CTOR";
    }

    ~FileRemover() {
        qDebug() << "Claude::FileRemover DTOR";
        worker_.request_stop();
        //condition_.notify_one();
    }

    void removeFiles(const IntQStringMap& rowPathMap)
    {
        worker_ = std::jthread([this, rowPathMap](const std::stop_token& st) {
            rmFiles(st, rowPathMap);
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
        worker_.request_stop();
    }

    //bool isFile(fs::directory_entry entry) {
    //    return entry.is_regular_file() || entry.is_symlink() ||
    //           entry.is_block_file() || entry.is_character_file() ||
    //           entry.is_fifo() || entry.is_socket() || entry.is_other();
    //}

    struct DeepDirCount {
        uint64_t files = 0;
        uint64_t folders = 0;
    };

    DeepDirCount deepCount(const fs::path& path) {
        DeepDirCount count;
        try {
            for (const auto& entry : rec_dir_it(path, dir_opts::skip_permission_denied)) {
                try {
                    if (entry.is_directory())
                        count.folders++;
                    else {
                        count.files++;
                    }
                }
                catch (const fs::filesystem_error& ex) {
                    std::cerr << "ERROR accessing " << entry.path() << ":\n"
                              << ex.what() << std::endl;
                }
            }
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "ERROR accessing dir " << path << ":\n"
                      << ex.what() << std::endl;
        }
        return count;
    }

private:
    void rmFiles(const std::stop_token& st, const IntQStringMap& rowPathMap)
    {
        set_thread_name("ClaudeFileRemover");
        const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        qDebug() << "Claude: rmFiles: my thread id:" << get_readable_thread_id() << " hash:" << tid;
        auto success = true;
        auto nbrDel = uint64_t(0);
        for (const auto& [row, path] : rowPathMap) {
            if (st.stop_requested())
                break;
            const auto pathStd = path.toStdString();
            bool rmOk = false;
            const auto isDir = fs::is_directory(pathStd);
            uint64_t size = 0;
            DeepDirCount ndel;
            try {
                if (fs::is_directory(pathStd)) {
                    ndel = deepCount(pathStd);
                    ndel.folders++; // count the root folder
                    rmOk = fs::remove_all(pathStd) > 0;
                }
                else {
                    ndel = { 1, 0 };
                    size = fs::file_size(pathStd); // MUST BE DONE BEFORE REMOVAL
                    rmOk = fs::remove(pathStd);
                }
            }
            catch (const fs::filesystem_error& e) {
                rmOk = false;
                qDebug() << "Remove ERROR:" << e.what();
            }
            if (rmOk)
                nbrDel += ndel.files + ndel.folders;
            else
                success = false;
            // Report progress
            const QString kind = isDir ? "FOLDER" : "file";
            std::lock_guard<std::mutex> lock(mutex_);
            if (progressCallback_) {
                progressCallback_(row, path, size, rmOk, nbrDel);
                //qDebug() << "rmOk:" << rmOk << "removed:" << kind << pathQstr;
            }
        }
        if (completionCallback_) {
            completionCallback_(success);
        }
        qDebug() << "Claude: thread FUNCTION finished, NBR DELETED:" << nbrDel;
    }

    std::jthread worker_;
    std::mutex mutex_;
    //std::queue<IntFsPathPair> removalQueue_;
    //std::condition_variable_any condition_;
    ProgressCallback progressCallback_;
    CompletionCallback completionCallback_;
};
}