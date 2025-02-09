#pragma once

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
    void queueForRemoval(const fs::path& path) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            removalQueue_.push(path);
        }
        condition_.notify_one();
    }

    // Set callback for progress updates
    void setProgressCallback(std::function<void(const fs::path&, bool)> callback) {
        //std::lock_guard<std::mutex> lock(mutex_);
        progressCallback_ = std::move(callback);
    }

private:
    void processQueue(const std::stop_token& st) {
        set_thread_name("ClaudeFileRemover");
        const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        qDebug() << "processQueue: my thread id:" << get_readable_thread_id() << " hash:" << tid;
        while (!st.stop_requested()) {
            fs::path path;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this, &st]() {
                    return !removalQueue_.empty() || st.stop_requested();
                    });
                path = removalQueue_.front();
                removalQueue_.pop();
            }
            bool success = false;
            try {
                if (fs::is_directory(path)) {
                    success = fs::remove_all(path) > 0;
                }
                else {
                    success = fs::remove(path);
                }
            }
            catch (const fs::filesystem_error& e) {
                success = false;
                qDebug() << "remove path:" << path << " error:" << e.what();
            }

            // Report progress
            std::lock_guard<std::mutex> lock(mutex_);
            if (progressCallback_) {
                progressCallback_(path, success);
                qDebug() << "remove path:" << path << " success:" << success;
            }
        }
    }

    std::jthread worker_;
    std::queue<fs::path> removalQueue_;
    std::mutex mutex_;
    std::condition_variable_any condition_;
    std::function<void(const fs::path&, bool)> progressCallback_;
};
}