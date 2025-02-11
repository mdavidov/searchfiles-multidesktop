#pragma once

#include "common.h"
#include "get_readable_thread_id.hpp"
#include "set_thread_name.hpp"
#include <filesystem>
#include <functional>
#include <thread>
#include <QMetaObject>
#include <QString>
#include <QStringList>

namespace fs = std::filesystem;

namespace AmzQ
{

    class FileRemover
    {
    public:
        // Callback types for progress and completion
        using ProgressCallback = std::function<void(int row, const QString&, uint64_t size, bool success)>;
        using CompletionCallback = std::function<void(bool)>;

        FileRemover(QObject* uiObject) : m_uiObject(uiObject) {
            qDebug() << "AmzQ::FileRemover CTOR: main thread is NOT BLOCKED until dtor is called";
        }
        ~FileRemover() {
            qDebug() << "AmzQ::FileRemover DTOR: JOIN is called, main thread is BLOCKED waiting for worker thread to finish";
        }

        void removeFilesAndFolders02(
            const IntQStringMap& paths,
            ProgressCallback progressCb,
            CompletionCallback completionCb
        ) {
            // Store callbacks as member variables or use shared_ptr to extend lifetime
            m_progressCb = std::move(progressCb);
            m_completionCb = std::move(completionCb);
            m_paths = paths;  // Store paths as member variable

            // Create jthread with captures by reference to class members
            m_worker = std::jthread(
                [this](std::stop_token stoken)
                {
                    qDebug() << "AmzQ::FileRemover: worker thread FUNCTION started";
                    bool success = true;
                    set_thread_name("AmzQFileRemover");
                    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
                    qDebug() << "removeFilesAndFolders02: my thread id:" << get_readable_thread_id() << " hash:" << tid;

                    for (const auto& path : m_paths) {
                        if (stoken.stop_requested()) {
                            break;
                        }
                        const fs::path fsPath = path.second.toStdString();
                        try {
                            bool rmOk = false;
                            uint64_t size = 0;
                            const bool isDir = fs::is_directory(fsPath);
                            if (isDir) {
                                rmOk = fs::remove_all(fsPath) > 0;
                            }
                            else {
                                size = fs::file_size(fsPath);
                                rmOk = fs::remove(fsPath);
                            }
                            QMetaObject::invokeMethod(m_uiObject, [this, path, size, rmOk]() {
                                m_progressCb(path.first, path.second, size, rmOk);
                            }, Qt::QueuedConnection);
                        }
                        catch (const fs::filesystem_error& e) {
                            success = false;
                            // Handle error by notifying UI
                            const QString errorMsg = QString("ERROR %1").arg(QString::fromStdString(e.what()));
                            qDebug() << errorMsg;
                            QMetaObject::invokeMethod(m_uiObject, [this, errorMsg]() {
                                m_progressCb(0, errorMsg, 0, false);
                            }, Qt::QueuedConnection);
                        }
                    }
                    QMetaObject::invokeMethod(m_uiObject, [this, success]() {
                        m_completionCb(success);
                    }, Qt::QueuedConnection);
                    qDebug() << "AmzQ::FileRemover: worker thread FUNCTION finished";
                });
        }

        //void removeFilesAndFolders01(
        //    const QStringList& paths,
        //    ProgressCallback progressCb,
        //    CompletionCallback completionCb
        //) {
        //    // Create a jthread for the removal operation
        //    m_worker = std::jthread(
        //        [this, paths, progressCb, completionCb](std::stop_token stoken) {
        //            bool success = true;
        //
        //            for (const auto& path : paths) {
        //                // Check if stop was requested
        //                if (stoken.stop_requested()) {
        //                    break;
        //                }
        //                fs::path fsPath = path.toStdString();
        //                try {
        //                    // Update UI with current file being processed
        //                    QMetaObject::invokeMethod(m_uiObject, [progressCb, path]() {
        //                        progressCb(path);
        //                        }, Qt::QueuedConnection);
        //
        //                    if (fs::exists(fsPath)) {
        //                        if (fs::is_directory(fsPath)) {
        //                            fs::remove_all(fsPath);
        //                        }
        //                        else {
        //                            fs::remove(fsPath);
        //                        }
        //                    }
        //                }
        //                catch (const fs::filesystem_error& e) {
        //                    success = false;
        //                    // Handle error by notifying UI
        //                    QString errorMsg = QString("Error removing %1: %2")
        //                        .arg(path)
        //                        .arg(QString::fromStdString(e.what()));
        //
        //                    QMetaObject::invokeMethod(m_uiObject, [progressCb, errorMsg]() {
        //                        progressCb(errorMsg);
        //                        }, Qt::QueuedConnection);
        //                }
        //            }
        //
        //            // Notify completion on UI thread
        //            QMetaObject::invokeMethod(m_uiObject, [completionCb, success]() {
        //                completionCb(success);
        //                }, Qt::QueuedConnection);
        //            });
        //}

        void stop() {
            m_worker.request_stop();
            //if (m_worker.joinable()) {
            //    m_worker.request_stop();
            //    m_worker.join();
            //}
        }

    private:
        QObject* m_uiObject;
        std::jthread m_worker;
        IntQStringMap m_paths;
        ProgressCallback m_progressCb;
        CompletionCallback m_completionCb;
    };
}
