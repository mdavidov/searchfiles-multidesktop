#pragma once

#include "common.h"
#include "get_readable_thread_id.hpp"
#include "set_thread_name.hpp"
#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace fs = std::filesystem;
class QString;

namespace Claude
{
class FileRemover
{
public:
    FileRemover() : worker_([this](const std::stop_token& st) { processQueue(st); }) {}

    ~FileRemover() {
        worker_.request_stop();
        condition_.notify_one();
    }

    // Add a path to the removal queue
    void queueForRemoval(const IntFsPathPair& rowPathPair) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            removalQueue_.push(rowPathPair);
        }
        condition_.notify_one();
    }

    // Set callback for progress updates
    void setProgressCallback(std::function<void(int, const QString&, uint64_t, bool)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        progressCallback_ = std::move(callback);
    }

private:
    void processQueue(const std::stop_token& st) {
        set_thread_name("ClaudeFileRemover");
        const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        qDebug() << "processQueue: my thread id:" << get_readable_thread_id() << " hash:" << tid;
        while (!st.stop_requested() && !removalQueue_.empty()) {
            std::pair<int, fs::path> pair;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this, &st]() {
                    return !removalQueue_.empty() || st.stop_requested();
                    });
                pair = removalQueue_.front();
                removalQueue_.pop();
            }
            bool success = false;
            const auto isDir = fs::is_directory(pair.second);
            const auto size = isDir ? 0 : fs::file_size(pair.second);
            try {
                if (fs::is_directory(pair.second)) {
                    success = fs::remove_all(pair.second) > 0;
                }
                else {
                    success = fs::remove(pair.second);
                }
            }
            catch (const fs::filesystem_error& e) {
                success = false;
                qDebug() << "remove error:" << e.what() << "path:" << pair.second;
            }
            // Report progress
            const auto pathQstr = FsPathToQStr(pair.second);
            const QString kind = isDir ? "FOLDER" : "file";
            std::lock_guard<std::mutex> lock(mutex_);
            if (progressCallback_) {
                progressCallback_(pair.first, pathQstr, size, success);
                qDebug() << "success:" << success << "removed:" << kind << pathQstr;
            }
        }
    }

    std::jthread worker_;
    std::queue<IntFsPathPair> removalQueue_;
    std::mutex mutex_;
    std::condition_variable_any condition_;
    std::function<void(int, const QString&, uint64_t, bool)> progressCallback_;
};
}