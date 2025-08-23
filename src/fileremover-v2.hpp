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
#include <filesystem>
#include <functional>
#include <thread>
#include <QDebug>
#include <QMetaObject>
#include <QString>
#include <QStringList>

namespace fs = std::filesystem;

namespace Frv2
{

    /// @brief FileRemover v2 class for removing files and folders.
    /// It uses a std::jthread (C++20) to perform the removal in a separate thread.
    /// It provides a callback mechanism for progress updates and completion.
    /// The progress callback is called for each file/folder removed,
    /// and the completion callback is called when the removal is complete.
    /// @author Milivoj (Mike) DAVIDOV
    ///
    class FileRemover
    {
    public:
        // Callback types for progress and completion
        using ProgressCallback = std::function<void(int row, const QString&, uint64_t size, bool success, uint64_t nbrDel)>;
        using CompletionCallback = std::function<void(bool)>;

        explicit FileRemover(QObject* uiObject) : m_uiObject(uiObject) {
        }
        ~FileRemover() {
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
                    qDebug() << "Frv2::FileRemover: worker thread function STARTED";
                    auto success = true;
                    auto nbrDel = uint64_t(0);
                    set_thread_name("Frv2FileRemover");
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
                                const auto nd = fs::remove_all(fsPath);
                                rmOk = (nd > 0);
                                nbrDel += nd;
                            }
                            else {
                                size = fs::file_size(fsPath); // MUST BE DONE BEFORE REMOVAL
                                rmOk = fs::remove(fsPath);
                                ++nbrDel;
                            }
                            QMetaObject::invokeMethod(m_uiObject, [this, row, path, size, rmOk, nbrDel]() {
                                m_progressCb(row, path, size, rmOk, nbrDel);
                            }, Qt::QueuedConnection);
                        }
                        catch (const fs::filesystem_error& e) {
                            success = false;
                            // Handle error by notifying UI
                            const QString errMsg = "Remove ERROR: " + QString(e.what());
                            qDebug() << errMsg;
                            QMetaObject::invokeMethod(m_uiObject, [this, errMsg]() {
                                m_progressCb(0, errMsg, 0, false, 0);
                            }, Qt::QueuedConnection);
                        }
                    }
                    QMetaObject::invokeMethod(m_uiObject, [this, success]() {
                        m_completionCb(success);
                    }, Qt::QueuedConnection);
                    qDebug() << "Frv2::FileRemover: worker thread function FINISHED";
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
