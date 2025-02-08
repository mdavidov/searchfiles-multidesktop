#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

class FileRemover {
public:
    FileRemover() : worker_([this](const std::stop_token& st) { processQueue(st); }) {}

    ~FileRemover() {
        worker_.request_stop();
        condition_.notify_one();
    }

    // Add a path to the removal queue
    void queueForRemoval(const std::filesystem::path& path) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            removalQueue_.push(path);
        }
        condition_.notify_one();
    }

    // Set callback for progress updates
    void setProgressCallback(std::function<void(const std::filesystem::path&, bool)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        progressCallback_ = std::move(callback);
    }

private:
    void processQueue(const std::stop_token& st) {
        while (!st.stop_requested()) {
            std::filesystem::path path;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this, &st]() {
                    return !removalQueue_.empty() || st.stop_requested();
                    });

                if (st.stop_requested()) {
                    return;
                }

                path = removalQueue_.front();
                removalQueue_.pop();
            }

            bool success = false;
            try {
                if (std::filesystem::is_directory(path)) {
                    success = std::filesystem::remove_all(path) > 0;
                }
                else {
                    success = std::filesystem::remove(path);
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                success = false;
            }

            // Report progress
            std::lock_guard<std::mutex> lock(mutex_);
            if (progressCallback_) {
                progressCallback_(path, success);
            }
        }
    }

    std::jthread worker_;
    std::queue<std::filesystem::path> removalQueue_;
    std::mutex mutex_;
    std::condition_variable_any condition_;
    std::function<void(const std::filesystem::path&, bool)> progressCallback_;
};
