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
using ProgressCallback = std::function<void(int, const QString&, uint64_t, bool)>;
using CompletionCallback = std::function<void(bool)>;

class FileRemover
{
public:
    FileRemover() {
        qDebug() << "Claude::FileRemover CTOR: main thread is NOT BLOCKED until dtor is called";
    }

    ~FileRemover() {
        qDebug() << "Claude::FileRemover DTOR: JOIN is called, main thread is BLOCKED until thread FUNCTION finishes, which should have happened already";
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

private:
    void rmFiles(const std::stop_token& st, const IntQStringMap& rowPathMap)
    {
        qDebug() << "Claude: worker thread FUNCTION started";
        set_thread_name("ClaudeFileRemover");
        const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        qDebug() << "Claude: rmFiles: my thread id:" << get_readable_thread_id() << " hash:" << tid;
        auto success = true;
        auto nbrDel = 0;
        for (const auto& [row, path] : rowPathMap) {
            if (st.stop_requested())
                break;
            const auto pathStd = path.toStdString();
            bool rmOk = false;
            const auto isDir = fs::is_directory(pathStd);
            uint64_t size = 0;
            try {
                if (fs::is_directory(pathStd)) {
                    rmOk = fs::remove_all(pathStd) > 0;
                }
                else {
                    size = fs::file_size(pathStd); // MUST BE DONE BEFORE REMOVAL
                    rmOk = fs::remove(pathStd);
                }
            }
            catch (const fs::filesystem_error& e) {
                rmOk = false;
                qDebug() << "Claude: remove error:" << e.what() << "pathStd:" << pathStd;
            }
            if (!rmOk)
                success = false;
            else
                ++nbrDel;
            // Report progress
            const QString kind = isDir ? "FOLDER" : "file";
            std::lock_guard<std::mutex> lock(mutex_);
            if (progressCallback_) {
                progressCallback_(row, path, size, rmOk);
                //qDebug() << "rmOk:" << rmOk << "removed:" << kind << pathQstr;
                //std::this_thread::yield();
                std::this_thread::sleep_for(std::chrono::nanoseconds(5));
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