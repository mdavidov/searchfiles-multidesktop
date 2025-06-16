#pragma once

#include "common.h"
#include "get_readable_thread_id.hpp"
#include "set_thread_name.hpp"
#include <filesystem>
#include <functional>
#include <QMetaObject>
#include <QString>
#include <QStringList>
#include <thread>

namespace fs = std::filesystem;

namespace AmzQ
{

    class FileRemover
    {
    public:
        // Callback types for progress and completion
        using ProgressCallback = std::function<void(int row, const QString&, uint64_t size, bool success)>;
        using CompletionCallback = std::function<void(bool)>;

        explicit FileRemover(QObject* uiObject) : m_uiObject(uiObject) {
            //qDebug() << "AmzQ::FileRemover CTOR";
        }
        ~FileRemover() {
            //qDebug() << "AmzQ::FileRemover DTOR";
        }

        void removeFilesAndFolders02(
            const IntQStringMap& rowPathMap,
            ProgressCallback progressCb,
            CompletionCallback completionCb
        ) {
            // Store callbacks as member variables or use shared_ptr to extend lifetime
            m_progressCb = std::move(progressCb);
            m_completionCb = std::move(completionCb);
            m_rowPathMap = rowPathMap;  // Store paths as member variable

            // Create jthread with captures by reference to class members
            m_worker = std::jthread(
                [this](std::stop_token stoken)
                {
                    qDebug() << "AmzQ::FileRemover: worker thread FUNCTION started";
                    bool success = true;
                    set_thread_name("AmzQFileRemover");
                    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
                    qDebug() << "removeFilesAndFolders02: my thread id:" << get_readable_thread_id() << " hash:" << tid;

                    for (const auto& [row, path] : m_rowPathMap) {
                        if (stoken.stop_requested()) {
                            break;
                        }
                        const fs::path fsPath = path.toStdString();
                        try {
                            bool rmOk = false;
                            uint64_t size = 0;
                            const bool isDir = fs::is_directory(fsPath);
                            if (isDir) {
                                rmOk = fs::remove_all(fsPath) > 0;
                            }
                            else {
                                size = fs::file_size(fsPath); // MUST BE DONE BEFORE REMOVAL
                                rmOk = fs::remove(fsPath);
                            }
                            QMetaObject::invokeMethod(m_uiObject, [this, row, path, size, rmOk]() {
                                m_progressCb(row, path, size, rmOk);
                            }, Qt::QueuedConnection);
                        }
                        catch (const fs::filesystem_error& e) {
                            success = false;
                            // Handle error by notifying UI
                            const QString errMsg = "Remove ERROR: " + QString(e.what());
                            qDebug() << errMsg;
                            QMetaObject::invokeMethod(m_uiObject, [this, errMsg]() {
                                m_progressCb(0, errMsg, 0, false);
                            }, Qt::QueuedConnection);
                        }
                    }
                    QMetaObject::invokeMethod(m_uiObject, [this, success]() {
                        m_completionCb(success);
                    }, Qt::QueuedConnection);
                    qDebug() << "AmzQ::FileRemover: worker thread FUNCTION finished";
                });
        }

        void stop() {
            m_worker.request_stop();
        }

    private:
        QObject* m_uiObject;
        std::jthread m_worker;
        IntQStringMap m_rowPathMap;
        ProgressCallback m_progressCb;
        CompletionCallback m_completionCb;
    };
}
