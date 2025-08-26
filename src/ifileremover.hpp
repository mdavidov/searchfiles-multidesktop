#pragma once
//
// Copyright (c) Milivoj (Mike) DAVIDOV
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
#include "common.hpp"

namespace mmd
{
// Callback types for progress and completion
using ProgressCallback = std::function<void(int row, const QString& fsItemPath, uint64_t size, bool success, uint64_t nbrDel)>;
using CompletionCallback = std::function<void(bool)>;

/// @brief Interface for file and folder removal.
/// It defines the contract for file removal implementations.
/// Implementations should provide methods to remove files and folders,
/// and to report progress and completion.
/// This allows for different removal strategies (e.g., Frv2, Frv3)
/// to be used interchangeably.
/// @author Milivoj (Mike) DAVIDOV
///
class IFileRemover
{
public:
    virtual ~IFileRemover() = default;

    /// @brief Remove files and folders based on the provided row-path map.
    /// @param rowPathMap Map of row indices to file/folder paths.
    /// @param progressCb Callback for progress updates.
    /// @param completionCb Callback for completion notification.
    ///
    virtual void removeFilesAndFolders(
        const IntQStringMap& rowPathMap,
        mmd::ProgressCallback progressCb,
        mmd::CompletionCallback completionCb
    ) = 0;

    /// @brief Request to stop the ongoing removal operation.
    /// Each implementation must be able to stop the current
    /// removal procedure when requested.
    ///
    virtual void stop() = 0;
};

} // namespace mmd
