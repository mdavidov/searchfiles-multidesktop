#include <filesystem>
#include <functional>
#include <thread>
#include <QString>
#include <QStringList>


class FileRemover
{
public:
    // Callback types for progress and completion
    using ProgressCallback = std::function<void(const QString&)>;
    using CompletionCallback = std::function<void(bool)>;

    FileRemover(QObject* uiObject) : m_uiObject(uiObject) {}

    void removeFilesAndFolders(
        const QStringList& paths,
        ProgressCallback progressCb,
        CompletionCallback completionCb
    ) {
        // Store callbacks as member variables or use shared_ptr to extend lifetime
        m_progressCb = std::move(progressCb);
        m_completionCb = std::move(completionCb);
        m_paths = paths;  // Store paths as member variable

        // Create jthread with captures by reference to class members
        m_worker = std::jthread(
            [this](std::stop_token stoken) {
                bool success = true;

                for (const auto& path : m_paths) {
                    if (stoken.stop_requested()) {
                        break;
                    }
                    std::filesystem::path fsPath = path.toStdString();
                    try {
                        QMetaObject::invokeMethod(m_uiObject, [this, path]() {
                            m_progressCb(path);
                            }, Qt::QueuedConnection);
                        
                        if (std::filesystem::exists(fsPath)) {
                            if (std::filesystem::is_directory(fsPath)) {
                                std::filesystem::remove_all(fsPath);
                            }
                            else {
                                std::filesystem::remove(fsPath);
                            }
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        success = false;
                        // Handle error by notifying UI
                        QString errorMsg = QString("Error removing %1: %2")
                            .arg(path)
                            .arg(QString::fromStdString(e.what()));

                        QMetaObject::invokeMethod(m_uiObject, [this, errorMsg]() {
                            m_progressCb(errorMsg);
                            }, Qt::QueuedConnection);
                    }
                }

                QMetaObject::invokeMethod(m_uiObject, [this, success]() {
                    m_completionCb(success);
                    }, Qt::QueuedConnection);
            });
    }

    void removeFilesAndFolders01(
        const QStringList& paths,
        ProgressCallback progressCb,
        CompletionCallback completionCb
    ) {
        // Create a jthread for the removal operation
        m_worker = std::jthread(
            [this, paths, progressCb, completionCb](std::stop_token stoken) {
                bool success = true;

                for (const auto& path : paths) {
                    // Check if stop was requested
                    if (stoken.stop_requested()) {
                        break;
                    }
                    std::filesystem::path fsPath = path.toStdString();
                    try {
                        // Update UI with current file being processed
                        QMetaObject::invokeMethod(m_uiObject, [progressCb, path]() {
                            progressCb(path);
                            }, Qt::QueuedConnection);

                        if (std::filesystem::exists(fsPath)) {
                            if (std::filesystem::is_directory(fsPath)) {
                                std::filesystem::remove_all(fsPath);
                            }
                            else {
                                std::filesystem::remove(fsPath);
                            }
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        success = false;
                        // Handle error by notifying UI
                        QString errorMsg = QString("Error removing %1: %2")
                            .arg(path)
                            .arg(QString::fromStdString(e.what()));
    
                        QMetaObject::invokeMethod(m_uiObject, [progressCb, errorMsg]() {
                            progressCb(errorMsg);
                            }, Qt::QueuedConnection);
                    }
                }

                // Notify completion on UI thread
                QMetaObject::invokeMethod(m_uiObject, [completionCb, success]() {
                    completionCb(success);
                    }, Qt::QueuedConnection);
                });
    }

    void stopRemoval() {
        if (m_worker.joinable()) {
            m_worker.request_stop();
            m_worker.join();
        }
    }

private:
    QObject* m_uiObject;
    std::jthread m_worker;
    QStringList m_paths;
    ProgressCallback m_progressCb;
    CompletionCallback m_completionCb;
};
