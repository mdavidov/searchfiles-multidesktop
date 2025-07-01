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

#include "precompiled.h"
#include "folderscanner.hpp"
#include "scanparams.hpp"
#include <mutex>
#include <chrono>
#include <thread>
#include <shared_mutex>
#include <queue>
#include <utility>
#include <QApplication>
#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include <QTimer>

namespace Devonline
{
bool isSymbolic(const QFileInfo& info)
{
#ifdef Q_OS_WIN
    return //isAppExecutionAlias(info.absoluteFilePath()) ||
           //isWindowsSymlink(info.absoluteFilePath()) ||
           info.isShortcut() || info.isSymbolicLink() || info.isJunction();
#else
    return info.isAlias() || info.isSymLink() || info.isSymbolicLink();
#endif
}

FolderScanner::FolderScanner(QObject* parent)
: QObject(parent), stopped(false), dirCount(0), foundCount(0), foundSize(0), symlinkCount(0), totCount(0), totSize(0)
{
    prevEvents = 0;
    eventsTimer.start();
    prevProgress = 0;
    progressTimer.start();
}

void FolderScanner::reportProgress(const QString& path, bool doit /*= false*/) {
    try {
        const auto elapsed = progressTimer.elapsed();
        const auto diff = elapsed - prevProgress;
        if (doit || diff >= 500) {  // msec
            prevProgress = elapsed;
            if (!stopped)
                emit progressUpdate(path, totCount, totSize);
            else
                qDebug() << "FolderScanner::reportProgress STOPPING";
        }
    }
    catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
    catch (...) { qDebug() << "caught ... EXCEPTION"; }
}

void FolderScanner::processEvents()
{
    const auto elapsed = eventsTimer.elapsed();
    const auto diff = elapsed - prevEvents;
    if (diff >= 500) {  // msec
        prevEvents = elapsed;
        qApp->processEvents(QEventLoop::AllEvents, 200);
    }
}

void FolderScanner::stop() {
    try {
        std::unique_lock<std::shared_mutex> lock(mutex);
        stopped = true;
    }
    catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
    catch (...) { qDebug() << "caught ... EXCEPTION"; }
}

bool FolderScanner::isStopped() const {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return stopped;
}

bool FolderScanner::appendOrExcludeItem(const QString& /*dirPath*/, const QFileInfo& info)
{
    try {
        const auto filePath = QDir::fromNativeSeparators(info.absoluteFilePath());
        const auto isSymlink = isSymbolic(info);
        const auto isDir = info.isDir() && !isSymlink;
        const auto isFile = info.isFile() && !isSymlink;
        if (!params.exclFolderPatterns.empty() &&
                stringContainsAnyWord(filePath, params.exclFolderPatterns)) {
            return false;
        }
        if (isFile) {
            if (!params.exclFilePatterns.empty() &&
                    stringContainsAnyWord(info.fileName(), params.exclFilePatterns)) {
                return false;
            }
            if (!params.exclusionWords.empty() &&
                    fileContainsAnyWordChunked(filePath, params.exclusionWords)) {
                return false;
            }
        }
        auto toAppend = false;
        if (params.searchWords.empty()) {
            if (isSymlink)
                toAppend = params.inclSymlinks;
            else if (isDir)
                toAppend = params.inclFolders;
        }
        if (isFile && params.inclFiles) {
            toAppend = params.searchWords.empty() ||
                fileContainsAllWordsChunked(filePath, params.searchWords);
        }
        if (toAppend) {
            if (isSymlink)
                symlinkCount++;
            else if (isDir)
                dirCount++;
            else if (isFile) {
                foundCount++;
                foundSize += (quint64)info.size();
            }
        }
        return toAppend;
    }
    catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
    catch (...) { qDebug() << "caught ... EXCEPTION"; }
    return false;
}

void FolderScanner::zeroCounters()
{
    dirCount = 0;
    foundCount = 0;
    foundSize = 0;
    totCount = 0;
    totSize = 0;
}

void FolderScanner::getAllDirs(const QString& dirPath, QFileInfoList& infos)
{
    // Don't need files and not following symlinks
    // QDir::AllDirs means "don't apply name filters to directories"
    // Because we want to traverse the whole dir structure (except maybe hidden)
    QDir dir(dirPath);
    auto filters = QDir::Dirs | QDir::AllDirs | QDir::Drives | QDir::System | QDir::NoSymLinks | QDir::NoDotAndDotDot;
    if (!params.exclHidden)
        filters |= QDir::Hidden;
    if (!stopped)
        infos = dir.entryInfoList(filters);
}

void FolderScanner::getFileInfos(const QString& dirPath, QFileInfoList& infos) /*const*/
{
    if (stopped)
        return;
    QDir dir(dirPath);
    if (!params.nameFilters.empty()) {
        dir.setNameFilters(params.nameFilters);
        dir.setFilter(params.itemTypeFilter);
        infos = dir.entryInfoList();
    }
    else {
        getAllItems(dirPath, infos);
    }
}

void FolderScanner::getAllItems(const QString& path, QFileInfoList& infos) const
{
    if (stopped)
        return;
    const QDir dir = QDir(path);
    const auto filters = QDir::AllEntries | QDir::AllDirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
    infos = dir.entryInfoList(filters);
}

quint64 FolderScanner::combinedSize(const QFileInfoList& infos)
{
    quint64 size = 0;
    for (const auto& info : infos) {
        //processEvents();
        if (stopped)
            return 0;
        size += (quint64)info.size();
    }
    return size;
}

// Not necessary: void FolderScanner::updateTotals(const QString& path)
//{
//    QFileInfoList infos;
//    getAllItems(path, infos);
//    totCount += quint64(infos.count());
//    totSize += combinedSize(infos);
//}

bool FolderScanner::stringContainsAllWords(const QString& str, const QStringList& words)
{
    if (str.isEmpty() || words.empty())
        return false;
    for (const auto& word : words) {
        if (stopped)
            return false;
        if (!str.contains(word, params.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

bool FolderScanner::stringContainsAnyWord(const QString& str, const QStringList& words)
{
    if (str.isEmpty() || words.empty())
        return false;
    for (const auto& word : words) {
        if (stopped)
            return false;
        if (str.contains(word, params.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool FolderScanner::fileContainsAllWordsChunked(const QString& filePath, const QStringList& words)
{
    QFile file(filePath);
    if (file.size() == 0 || QFileInfo(file).fileName() == ".DS_Store")
        return false;
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file, error" << file.errorString();
        return false;
    }
    static constexpr qint64 CHUNK_SIZE = 200 * 1024 * 1024;
    while (!file.atEnd() && !stopped) {
        const auto rsize = std::min(file.size() - file.pos(), CHUNK_SIZE);
        const auto chunk = (rsize > 0) ? QString::fromUtf8(file.read(rsize)) : QString();
        if (chunk.isEmpty()) {
            return false;
        }
        if (!stringContainsAllWords(chunk, words)) {
            return false;
        }
    }
    return true;
}

bool FolderScanner::fileContainsAnyWordChunked(const QString& filePath, const QStringList& words)
{
    QFile file(filePath);
    if (file.size() == 0 || QFileInfo(file).fileName() == ".DS_Store")
        return false;
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file, error" << file.errorString();
        return false;
    }
    static constexpr qint64 CHUNK_SIZE = 200 * 1024 * 1024;
    while (!file.atEnd() && !stopped) {
        const auto rsize = std::min(file.size() - file.pos(), CHUNK_SIZE);
        const auto chunk = (rsize > 0) ? QString::fromUtf8(file.read(rsize)) : QString();
        if (chunk.isEmpty()) {
            return false;
        }
        if (stringContainsAnyWord(chunk, words)) {
            return true;
        }
    }
    return false;
}

void FolderScanner::deepScan(const QString& startPath, const int maxDepth)
{
    try {
        stopped = false;
        zeroCounters();
        QString lastPath;
        
        QQueue<QPair<QString, int>> dirQ;
        dirQ.enqueue({ startPath, 0 });
        
        while (!dirQ.empty()) {
            if (stopped) {
                return;
            }
            processEvents();
            const auto [dirPath, currDepth] = dirQ.dequeue();
            reportProgress(dirPath + QDir::separator());
            QFileInfoList dirInfos;
            getAllDirs(dirPath, dirInfos);
            for (const auto& dir : dirInfos) {
                lastPath = dirPath;
                if (stopped) {
                    return;
                }
                processEvents();
                const auto dPath = dir.absoluteFilePath();
                if ((maxDepth < 0 || currDepth < maxDepth) &&
                    (params.exclFolderPatterns.empty() || !stringContainsAnyWord(dPath, params.exclFolderPatterns))) {
                        dirQ.enqueue({ dPath, currDepth + 1 });
                }
                reportProgress(dPath + QDir::separator());
            }
            // Not necessary: updateTotals(dirPath);
            
            QFileInfoList infos;
            getFileInfos(dirPath, infos);
            for (const auto& info : infos) {
                if (stopped) {
                    return;
                }
                processEvents();
                const auto filePath = info.absoluteFilePath();
                if (appendOrExcludeItem(filePath, info)) {
                    emit itemFound(filePath, info);
                }
                reportProgress(filePath);
                lastPath = filePath;
            }
        }
        if (!stopped) {
            reportProgress(lastPath, true);
            emit scanComplete();
        }
        stopped = true;
    }
    catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
    catch (...) { qDebug() << "caught ... EXCEPTION"; }
}

uint64pair FolderScanner::deepCountSize(const QString& startPath)
{
    quint64 count = 0;
    quint64 size = 0;
    try {
        stopped = false;
        QQueue<QString> dirQ;
        dirQ.enqueue(startPath);
        zeroCounters();
        QString lastPath;

        while (!dirQ.empty() && !stopped) {
            processEvents();
            const auto dirPath = dirQ.dequeue();
            lastPath = dirPath;
            QFileInfoList dirInfos;
            getAllDirs(dirPath, dirInfos);
            for (const auto& info : dirInfos) {
                //processEvents();
                if (stopped) {
                    emit scanCancelled();
                    return{ count, size };
                }
                if (info.isDir() && !isSymbolic(info)) {
                    dirQ.enqueue(info.absoluteFilePath());
                }
            }

            QFileInfoList infos;
            getAllItems(dirPath, infos);
            for (const auto& info : infos) {
                processEvents();
                if (stopped) {
                    emit scanCancelled();
                    return{ count, size };
                }
                ++count;
                size += (quint64)info.size();
                foundCount = count;
                foundSize = size;
                const auto filePath = info.absoluteFilePath();
                reportProgress(filePath);
                emit itemSized(filePath, info);
                lastPath = filePath;
            }
        }
        if (!stopped) {
            reportProgress(lastPath, true);
        }
    }
    catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
    catch (...) { qDebug() << "caught ... EXCEPTION"; }
    return{ count, size };
}

void FolderScanner::deepRemove(const IntQStringMap& rowPathMap)
{
    stopped = false;
    zeroCounters();
    quint64 nbrDeleted = 0;
    for (const auto& rowPath : rowPathMap)
    {
        try {
            processEvents();
            if (stopped)
                return;
            const auto path = rowPath.second;
            const auto info = QFileInfo(path);

            if (!info.isDir()) {
                // RM FILE or SYMLINK
                QFile file(path);
                const auto size = info.size();
                const auto rmok = file.remove();
                if (rmok) {
                    ++nbrDeleted;
                    emit itemRemoved(rowPath.first, 1, (quint64)size, nbrDeleted);
                }
            }
            else if (!isSymbolic(info)) {
                // RM DIR
                QDir dir(path);
                // Getting dir size (deepCountSize(path)) could be hugely time consuming
                const auto rmok = dir.removeRecursively();
                processEvents();
                if (rmok) {
                    ++nbrDeleted;
                    emit itemRemoved(rowPath.first, 1, 0, nbrDeleted);
                }
            }
        }
        catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
        catch (...) { qDebug() << "caught ... EXCEPTION"; }
    }
    stopped = true;
}

void FolderScanner::deepRemoveLimited(const IntQStringMap& rowPathMap, const int maxDepth)
{
    stopped = false;
    zeroCounters();
    quint64 nbrDeleted = 0;
    std::deque<QString> dirPaths;
    auto res = true;
    
    // Get all selected files and dirs; remove files and empty dirs; add still existing dirs to deque
    for (const auto& rowNpath : rowPathMap) {
        try {
            processEvents();
            if (stopped) {
                qDebug() << "deepRemoveLimited: STOPPED";
                emit removalCancelled();
                return;
            }
            const auto path = rowNpath.second;
            const auto info = QFileInfo(path);
            if (!doRemoveOneFile(info, rowNpath.first, nbrDeleted)) {
                res = false;
            }
            if (info.isDir() && !isSymbolic(info) && QFileInfo::exists(path)) {
                dirPaths.push_back(path);
            }
        }
        catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
        catch (...) { qDebug() << "caught ... EXCEPTION"; }
    }

    // For each dir that was selected (i.e. it's in rowPathMap):
    // deep remove its files until maxDepth is reached.
    if (maxDepth != 0) {  // maxDepth == -1 means unlimitted, >= 0 means limited
        for (const auto& dirPath : dirPaths) {
            deepRemLimitedImpl(dirPath, maxDepth, nbrDeleted);
        }
    }

    emit removalComplete(res);
    stopped = true;
}

bool FolderScanner::deepRemLimitedImpl(const QString& startPath, const int maxDepth, quint64& nbrDeleted)
{
    std::queue<std::pair<QString, int>> dirQ;
    dirQ.push({ startPath, 1 }); // start at level 1
    auto res = true;

    while (!dirQ.empty() && !stopped) {
        try {
            processEvents();
            const auto [dirPath, currDepth] = dirQ.front();
            dirQ.pop();
            QFileInfoList dirInfos;
            getAllDirs(dirPath, dirInfos);
            for (const auto& dir : dirInfos) {
                if (stopped) {
                    qDebug() << "deepRemLimitedImpl: STOPPED @ line 445";
                    emit removalCancelled();
                    return res;
                }
                if (maxDepth < 0 || currDepth < maxDepth) {
                    dirQ.push({ dir.absoluteFilePath(), currDepth + 1 });
                    qDebug() << "deepRemLimitedImpl: dirQ.push" << dir.absoluteFilePath() << "currDepth" << currDepth << "maxDepth" << maxDepth;
                }
            }
            
            QFileInfoList infos;
            getFileInfos(dirPath, infos);
            for (const auto& info : infos) {
                processEvents();
                if (stopped) {
                    qDebug() << "deepRemLimitedImpl: STOPPED @ line 458";
                    emit removalCancelled();
                    return res;
                }
                if (!doRemoveOneFile(info, -1, nbrDeleted)) {  // not in the files table, so row = -1
                    res = false;
                }
            }
            // If the containing folder is empty, then remove it.
            const auto dir = QDir(dirPath);
            if (dir.isEmpty()) {
                dir.rmdir(dirPath);
            }
        }
        catch (const std::exception& ex) { qDebug() << "EXCEPTION: " << ex.what(); }
        catch (...) { qDebug() << "caught ... EXCEPTION"; }
    }
    return res;
}

bool FolderScanner::doRemoveOneFile(const QFileInfo& info, int row, quint64& nbrDeleted)
{
    auto res = true;
    if (!info.isDir()) {
        // RM FILE or SYMLINK
        QFile file(info.absoluteFilePath());
        const auto size = info.size();
        const auto rmok = file.remove();
        if (rmok) {
            ++nbrDeleted;
            emit itemRemoved(row, 1, (quint64)size, nbrDeleted); // not in the table, so row = -1
            qDebug() << "Removed" << info.absoluteFilePath() << "size" << size << "nbrDeleted" << nbrDeleted;
        }
        else {
            res = false;
            qDebug() << "FAILED to remove" << info.absoluteFilePath() << "size" << size << "nbrDeleted" << nbrDeleted;
        }
    }
    else {
        // RM DIR if it's empty
        res = rmEmptyDir(info.absoluteFilePath(), row, nbrDeleted);
    }
    return res;
}

bool FolderScanner::rmEmptyDir(const QString& dirPath, int row, quint64& nbrDeleted)
{
    // RM DIR if it's empty
    QDir dir(dirPath);
    if (!dir.isEmpty())
        return true;
    const auto rmok = dir.rmdir(dirPath);
    processEvents();
    if (rmok) {
        ++nbrDeleted;
        emit itemRemoved(row, 1, 0, nbrDeleted);
        qDebug() << "Removed" << dirPath << "nbrDeleted" << nbrDeleted;
        return true;
    }
    else {
        qDebug() << "FAILED to remove" << dirPath << "nbrDeleted" << nbrDeleted;
        return false;
    }
}

}
