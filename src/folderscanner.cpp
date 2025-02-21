#include "precompiled.h"
#include "folderscanner.hpp"
#include "scanparams.hpp"
#include <mutex>
#include <chrono>
#include <thread>
#include <shared_mutex>
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
    const auto elapsed = progressTimer.elapsed();
    const auto diff = elapsed - prevProgress;
    if (doit || diff >= 500) {  // msec
        prevProgress = elapsed;
        if (!stopped)
            emit progressUpdate(path, totCount, totSize);
    }
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
    std::unique_lock<std::shared_mutex> lock(mutex);
    stopped = true;
}

bool FolderScanner::isStopped() const {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return stopped;
}

bool FolderScanner::appendOrExcludeItem(const QString& dirPath, const QFileInfo& info)
{
    try {
        const auto filePath = QDir::fromNativeSeparators(info.absoluteFilePath());
        const auto isSymlink = isSymbolic(info);
        const auto isDir = info.isDir() && !isSymlink;
        const auto isFile = info.isFile() && !isSymlink;
        if (!params.exclFolderPatterns.empty() &&
                stringContainsAnyWord(dirPath, params.exclFolderPatterns)) {
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
        bool toAppend = false;
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
                foundSize += info.size();
            }
        }
        return toAppend;
    }
    catch (...) { Q_ASSERT(false); return false; } // tell the user?
}

void FolderScanner::getAllDirs(const QString& currPath, QFileInfoList& infos)
{
    // Don't need files and not following symlinks
    // QDir::AllDirs means "don't apply name filters to directories"
    // Because we want to traverse the whole dir structure (except maybe hidden)
    QDir currDir(currPath);
    auto filters = QDir::Dirs | QDir::AllDirs | QDir::Drives | QDir::System | QDir::NoSymLinks | QDir::NoDotAndDotDot;
    if (!params.exclHidden)
        filters |= QDir::Hidden;
    infos = currDir.entryInfoList(filters);
}

void FolderScanner::getFileInfos(const QString& currPath, QFileInfoList& infos) /*const*/
{
    if (stopped)
        return;
    QDir currDir(currPath);
    if (!params.nameFilters.empty()) {
        currDir.setNameFilters(params.nameFilters);
        currDir.setFilter(params.itemTypeFilter);
        infos = currDir.entryInfoList();
    }
    else {
        getAllItems(currPath, infos);
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
        size += info.size();
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

void FolderScanner::deepScan(const QString& startPath, const int maxDepth) {
    stopped = false;
    zeroCounters();
    QString lastPath;

    QQueue<QPair<QString, int>> dirQ;
    dirQ.enqueue({ startPath, 0 });

    while (!dirQ.empty() && !stopped) {
        processEvents();
        const auto [currPath, currDepth] = dirQ.dequeue();
        QFileInfoList dirInfos;
        getAllDirs(currPath, dirInfos);
        for (const auto& dir : dirInfos) {
            lastPath = currPath;
            //processEvents();
            if (stopped) {
                //emit scanCancelled();
                return;
            }
            if (maxDepth < 0 || currDepth < maxDepth) {
                dirQ.enqueue({ dir.absoluteFilePath(), currDepth + 1 });
            }
            reportProgress(currPath);
        }
        // Not necessary: updateTotals(currPath);

        QFileInfoList infos;
        getFileInfos(currPath, infos);
        for (const auto& info : infos) {
            processEvents();
            if (stopped) {
                //emit scanCancelled();
                return;
            }
            if (appendOrExcludeItem(currPath, info)) {
                emit itemFound(info.absoluteFilePath(), info);
            }
            else {
                reportProgress(currPath);
            }
        }
        lastPath = currPath;
        reportProgress(currPath);
    }
    if (!stopped) {
        reportProgress(lastPath, true);
        emit scanComplete();
    }
    stopped = true;
}

uint64pair FolderScanner::deepCountSize(const QString& startPath)
{
    stopped = false;
    QQueue<QString> dirQ;
    dirQ.enqueue(startPath);
    zeroCounters();
    QString lastPath;
    quint64 count = 0;
    quint64 size = 0;

    while (!dirQ.empty() && !stopped) {
        processEvents();
        const auto currPath = dirQ.dequeue();
        lastPath = currPath;
        QFileInfoList dirInfos;
        getAllDirs(currPath, dirInfos);
        for (const auto& dir : dirInfos) {
            //processEvents();
            if (stopped) {
                return{ count, size };
            }
            dirQ.enqueue(dir.absoluteFilePath());
            reportProgress(dir.absoluteFilePath());
        }

        QFileInfoList infos;
        getAllItems(currPath, infos);
        for (const auto& info : infos) {
            processEvents();
            if (stopped) {
                return{ count, size };
            }
            ++count;
            size += info.size();
            foundCount = count;
            foundSize = size;
            reportProgress(info.absoluteFilePath());
            //emit itemSized(info.absoluteFilePath(), info);
        }
        lastPath = currPath;
        reportProgress(currPath);
    }
    if (!stopped) {
        reportProgress(lastPath, true);
    }
    stopped = true;
    return{ count, size };
}

void FolderScanner::deepRemove(const IntQStringMap& rowPathMap)
{
    stopped = false;
    zeroCounters();
    int nbrDeleted = 0;
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
                    emit itemRemoved(rowPath.first, 1, size, nbrDeleted);
                }
            }
            else {
                // RM DIR
                QDir dir(path);
                // Getting dir size (deepCountSize(path)) could be way too time consuming
                const auto rmok = dir.removeRecursively();
                processEvents();
                if (rmok) {
                    ++nbrDeleted;
                    emit itemRemoved(rowPath.first, 1, 0, nbrDeleted);
                }
            }
        }
        catch (...) { Q_ASSERT(false); } // tell the user?
    }
    stopped = true;
}

void FolderScanner::zeroCounters()
{
    dirCount = 0;
    foundCount = 0;
    foundSize = 0;
    totCount = 0;
    totSize = 0;
}

}
