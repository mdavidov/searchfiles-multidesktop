/****************************************************************************
**
** Copyright (c) 2010 Milivoj (Mike) Davidov
** All rights reserved.
**
** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
** EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#pragma once

#include "config.h"
#include <QtCore/QtGlobal>
#include <QtCore/QString>

namespace Devonline
{
    /// @brief Type of a file system operation
    ///
    namespace Op
    {
        enum Type
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

        class Conv
        {
        public:
            static QString toString( Type opType)
            {
                switch (opType)
                {
                case getInfo:
                    return QObject::OvSk_FsOp_ANALYZING_TXT;

                case duplicate:
                    return QObject::OvSk_FsOp_DUPLICATING_TXT;

                case fastCopy:
                case simpleCopy:
                    return QObject::OvSk_FsOp_COPYING_TXT;

                case move:
                    return QObject::OvSk_FsOp_MOVING_TXT;

                case delete2Trash:
                    return QObject::OvSk_FsOp_DELETING_2TRASH_TXT;

                case deletePerm:
                    return QObject::OvSk_FsOp_DELETING_PERM_TXT;

                case shredPerm:
                    return QObject::OvSk_FsOp_SHREDDING_PERM_TXT;

                default:
                    return "";
                }
            }
        };
    }


    /// @brief Progress info
    ///
    class ProgressInfo
    {
    public:
        explicit ProgressInfo( Op::Type opType = Op::getInfo,
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

        Op::Type m_opType;
        quint64 m_nbrDirs;
        quint64 m_nbrFiles;
        quint64 m_nbrSymlinks;
        quint64 m_nbrFileBytes;
        quint64 m_nbrTotalBytes;
        quint64 m_nbrElapsedMilliSeconds;
    };

    // see Q_DECLARE_METATYPE(...) at the bottom of filesystemops.h file and qRegisterMetaType(...) in FileSystemOps ctor
}
