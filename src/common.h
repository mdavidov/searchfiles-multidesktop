/****************************************************************************
**
** Copyright (c) 2010 Milivoj (Mike) Davidov
** All rights reserved.
**
** THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND.
** COPYRIGHT HOLDER FURTHER DISCLAIMS ANY IMPLIED WARRANTIES, INCLUDING, WITHOUT
** LIMITATION, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF THE
** USE OR PERFORMANCE OF THE SOFTWARE REMAINS WITH YOU. SHOULD THE SOFTWARE
** PROVE DEFECTIVE, YOU (AND NOT COPYRIGHT HOLDER) ASSUME THE ENTIRE COST OF ALL
** NECESSARY SERVICING OR REPAIR.
**
** IN NO EVENT SHALL COPYRIGHT HOLDER OR ANYONE ELSE INVOLVED IN THE CREATION,
** PRODUCTION, MARKETING, DISTRIBUTION, OR DELIVERY OF THE SOFTWARE, BE LIABLE
** FOR ANY DAMAGES WHATSOEVER; INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS
** OF BUSINESS PROFITS, FOR BUSINESS INTERRUPTION, FOR LOSS OF BUSINESS
** INFORMATION, OR FOR OTHER MONETARY LOSS, ARISING OUT OF THE USE OF THE
** SOFTWARE OR THE INABILITY TO USE THE SOFTWARE, EVEN IF YOU HAVE BEEN NOTIFIED
** OF THE POSSIBILITY OF SUCH DAMAGES.
**
** IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
** CONSEQUENTIAL DAMAGES, OR FOR ANY DAMAGES WHATSOEVER, WHETHER IN A CONTRACT
** ACTION, NEGLIGENCE OR OTHER TORT ACTION, OR OTHER CLAIM OR ACTION, ARISING
** OUT OF, OR IN CONNECTION WITH, THE USE OR PERFORMANCE OF THE SOFTWARE OR
** DOCUMENTS AND OTHER INFORMATION PROVIDED TO YOU BY COPYRIGHT HOLDER, OR IN THE
** PROVISION OF, OR FAILURE TO PROVIDE, SERVICES OR INFORMATION.
**
****************************************************************************/
#ifndef _OVERSKYS_COMMON_H
#define _OVERSKYS_COMMON_H

#include "config.h"
#include <QtGlobal>
#include <QString>


namespace Overskys
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


#endif // _OVERSKYS_COMMON_H
