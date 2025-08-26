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
    class FileRemover : public mmd::IFileRemover
    {
    public:
        explicit FileRemover(QObject* uiObject) : m_uiObject(uiObject) {
        }
        ~FileRemover() override {
        }

        void removeFilesAndFolders(
            const IntQStringMap &rowPathMap,
            mmd::ProgressCallback progressCb,
            mmd::CompletionCallback completionCb) override
        {
            // Store callbacks as member variables or use shared_ptr to extend lifetime
            m_progressCb = std::move(progressCb);
            m_completionCb = std::move(completionCb);
            m_rowPathMap = rowPathMap; // Store paths as member variable

            // Create jthread with captures by reference to class members
            m_worker = std::jthread(
                [this](std::stop_token stoken)
                {
                    set_thread_name("Frv2FileRemover");
                    const auto handle = this->m_worker.native_handle(); (void)handle;
                    // qDebug() << "Frv2::FileRemover: Thread STARTED;  name: Frv2FileRemover" << "native_handle:" << handle;
                    auto success = true;
                    auto nbrDel = uint64_t(0);

                    for (const auto &[row, path] : m_rowPathMap) {
                        if (stoken.stop_requested()) {
                            break;
                        }
                        const fs::path fsPath = path.toStdString();
                        try
                        {
                            bool rmOk = false;
                            uint64_t size = 0;
                            std::error_code ec;
                            if (!fs::exists(fsPath, ec)) {
                                continue; // It could have been already removed
                            }
                            const bool isDir = fs::is_directory(fsPath);
                            if (isDir) {
                                const auto nd = fs::remove_all(fsPath, ec);
                                rmOk = (ec.value() == 0);
                                nbrDel += nd;
                            }
                            else {
                                size = fs::file_size(fsPath); // MUST BE DONE BEFORE REMOVAL
                                rmOk = fs::remove(fsPath, ec);
                                ++nbrDel;
                            }
                            if (ec.value() != 0) {
                                success = false;
                            }
                            QMetaObject::invokeMethod(m_uiObject,
                                [this, row, fsPath, size, rmOk, nbrDel](){ m_progressCb(row, fsPath.string().c_str(), size, rmOk, nbrDel); },
                                Qt::QueuedConnection);
                        }
                        catch (const fs::filesystem_error& e) {
                            success = false;
                            const QString errMsg = "EXCEPTION: " + QString(e.what());
                            qDebug() << errMsg;
                            QMetaObject::invokeMethod(m_uiObject,
                                [this, errMsg]() { m_progressCb(0, errMsg, 0, false, 0); },
                                Qt::QueuedConnection);
                        }
                    }
                    QMetaObject::invokeMethod(m_uiObject,
                        [this, success]() { m_completionCb(success); },
                        Qt::QueuedConnection);
                });
                m_worker.detach(); // Detach the thread to allow it to run independently
        }

        void stop() override {
            m_worker.request_stop();
        }

    private:
        QObject* m_uiObject;
        std::jthread m_worker;
        IntQStringMap m_rowPathMap;
        mmd::ProgressCallback m_progressCb;
        mmd::CompletionCallback m_completionCb;
    };
}
