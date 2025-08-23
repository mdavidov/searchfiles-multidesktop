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

#include "config.hpp"
#include <filesystem>
#include <map>
#include <utility>
#include <QtCore/QtGlobal>
#include <QtCore/QString>

using IntFsPathPair = std::pair<int, std::filesystem::path>;
using IntQStringMap = std::map<int, QString, std::greater<int>>;

inline QString FsPathToQStr(const std::filesystem::path& path)
{
#ifdef _WIN32
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromStdString(path.string());
#endif
}

namespace mmd
{
    /// @brief Type of a file system operation
    /// This enum defines the type of file system operation being performed.
    /// It is used to report progress and completion of the operation.
    /// @author Milivoj (Mike) DAVIDOV
    ///
    enum class FsOpType
    {
        getInfo,
        fastCopy,
        simpleCopy,
        duplicate,
        move,
        delete2Trash,
        deletePerm,
        shredPerm,
    };

    /// @brief  Conversion utility for FsOpType
    /// This struct provides a utility function to convert FsOpType to a QString.
    /// It is used to display the operation type in the UI.
    /// @author Milivoj (Mike) DAVIDOV
    ///
    struct Conv
    {
        static QString toString( FsOpType opType)
        {
            switch (opType)
            {
            case FsOpType::getInfo:
                return QObject::OvSk_FsOp_ANALYZING_TXT;

            case FsOpType::duplicate:
                return QObject::OvSk_FsOp_DUPLICATING_TXT;

            case FsOpType::fastCopy:
            case FsOpType::simpleCopy:
                return QObject::OvSk_FsOp_COPYING_TXT;

            case FsOpType::move:
                return QObject::OvSk_FsOp_MOVING_TXT;

            case FsOpType::delete2Trash:
                return QObject::OvSk_FsOp_DELETING_2TRASH_TXT;

            case FsOpType::deletePerm:
                return QObject::OvSk_FsOp_DELETING_PERM_TXT;

            case FsOpType::shredPerm:
                return QObject::OvSk_FsOp_SHREDDING_PERM_TXT;

            default:
                return "";
            }
        }
    };

    /// @brief Progress info structure
    /// This is used to report progress of file system operations.
    /// It contains the operation type, number of directories, files, symlinks,
    /// file bytes, total bytes, and elapsed time in milliseconds.
    /// It is used to update the UI with progress information.
    /// @author Milivoj (Mike) DAVIDOV
    ///
    struct ProgressInfo
    {
        explicit ProgressInfo( FsOpType opType = FsOpType::getInfo,
                               quint64 nbrDirs = 0,
                               quint64 nbrFiles = 0,
                               quint64 nbrSymLinks = 0,
                               quint64 nbrFileBytes = 0,
                               quint64 nbrTotalBytes = 0,
                               quint64 nbrElapsedMilliSeconds = 0)
            : m_opType( opType)
            , m_nbrDirs( nbrDirs)
            , m_nbrFiles( nbrFiles)
            , m_nbrSymlinks( nbrSymLinks)
            , m_nbrFileBytes( nbrFileBytes)
            , m_nbrTotalBytes( nbrTotalBytes)
            , m_nbrElapsedMilliSeconds( nbrElapsedMilliSeconds)
        {
        }

        ProgressInfo( const ProgressInfo & rhs)
            : m_opType( rhs.m_opType)
            , m_nbrDirs( rhs.m_nbrDirs)
            , m_nbrFiles( rhs.m_nbrFiles)
            , m_nbrSymlinks( rhs.m_nbrSymlinks)
            , m_nbrFileBytes( rhs.m_nbrFileBytes)
            , m_nbrTotalBytes( rhs.m_nbrTotalBytes)
            , m_nbrElapsedMilliSeconds( rhs.m_nbrElapsedMilliSeconds)
        {
        }

        ProgressInfo operator=( const ProgressInfo & rhs)
        {
            m_nbrDirs = rhs.m_nbrDirs;
            m_nbrFiles = rhs.m_nbrFiles;
            m_nbrSymlinks = rhs.m_nbrSymlinks;
            m_nbrFileBytes = rhs.m_nbrFileBytes;
            m_nbrTotalBytes = rhs.m_nbrTotalBytes;
            m_nbrElapsedMilliSeconds = rhs.m_nbrElapsedMilliSeconds;
            return *this;
        }

        ProgressInfo operator+=( const ProgressInfo & rhs)
        {
            m_nbrDirs += rhs.m_nbrDirs;
            m_nbrFiles += rhs.m_nbrFiles;
            m_nbrSymlinks += rhs.m_nbrSymlinks;
            m_nbrFileBytes += rhs.m_nbrFileBytes;
            m_nbrTotalBytes += rhs.m_nbrTotalBytes;
            m_nbrElapsedMilliSeconds += rhs.m_nbrElapsedMilliSeconds;
            return *this;
        }

        FsOpType m_opType;
        quint64 m_nbrDirs;
        quint64 m_nbrFiles;
        quint64 m_nbrSymlinks;
        quint64 m_nbrFileBytes;
        quint64 m_nbrTotalBytes;
        quint64 m_nbrElapsedMilliSeconds;
    };

    // see Q_DECLARE_METATYPE(...) at the bottom of filesystemops.h file and qRegisterMetaType(...) in FileSystemOps ctor
}
