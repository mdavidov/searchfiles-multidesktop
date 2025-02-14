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
FolderScanner::FolderScanner(QObject* parent)
: QObject(parent), stopped(false), foundCount(0), foundSize(0), totCount(0), totSize(0)
{
    eventsTimer.start();
}

void FolderScanner::reportProgress(const QString& path, bool doit /*= false*/) {
    const auto elapsed = progressTimer.elapsed();
    static qint64 prev = elapsed;  // initialised only once coz it's static
    const auto diff = elapsed - prev;
    if (doit || diff >= 200) {  // msec
        prev = elapsed;  // static var keeps its value for the next call
        if (!stopped)
            emit progressUpdate(path, foundCount, foundSize, totCount, totSize);
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

inline void FolderScanner::processEvents()
{
    const auto elapsed = eventsTimer.elapsed();
    // Like all static vars, local function statics are initialised only once,
    // first time the function is called.
    static qint64 prev = elapsed;
    const auto diff = elapsed - prev;
    if (diff >= 1'000) {  // msec
        // Static var prev keeps its value for the next call of this function.
        prev = elapsed;
        qApp->processEvents(QEventLoop::AllEvents, 400);
    }
}

bool FolderScanner::appendOrExcludeItem(const QString& dirPath, const QFileInfo& info)
{
    try {
        const auto filePath = QDir::fromNativeSeparators(info.absoluteFilePath());
        if (!params.exclFolderPatterns.empty() &&
                stringContainsAnyWord(dirPath, params.exclFolderPatterns)) {
            return false;
        }
        if (info.isFile()) {
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
            if ((info.isSymLink() || info.isSymbolicLink() || info.isShortcut()) && !info.isDir())
                toAppend = params.inclSymlinks;
            else if (info.isDir() && !info.isSymLink() && !info.isSymbolicLink() && !info.isShortcut())
                toAppend = params.inclFolders;
        }
        if (info.isFile() && params.inclFiles) {
            toAppend = params.searchWords.empty() ||
                fileContainsAllWordsChunked(filePath, params.searchWords);
        }
        if (toAppend) {
            foundCount++;
            foundSize += getItemSize(info);
        }
        return toAppend;
    }
    catch (...) { Q_ASSERT(false); return false; } // tell the user?
}

void FolderScanner::getAllDirs(const QString& currPath, QFileInfoList& infos) const
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
    const QDir dir = QDir(path);
    const auto filters = QDir::AllEntries | QDir::AllDirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
    infos = dir.entryInfoList(filters);
}

quint64 FolderScanner::getItemSize(const QFileInfo& info) const
{
    static constexpr qint64 DIR_SYMLINK_SIZE = 4 * 1024;
    return (info.isSymLink() || info.isSymbolicLink() || info.isShortcut()) ?
        DIR_SYMLINK_SIZE :
        quint64(info.size());
}

quint64 FolderScanner::combinedSize(const QFileInfoList& infos)
{
    quint64 size = 0;
    for (const auto& info : infos) {
        //processEvents();
        if (stopped)
            return 0;
        size += getItemSize(info);
    }
    return size;
}

void FolderScanner::updateTotals(const QString& path)
{
    QFileInfoList infos;
    getAllItems(path, infos);
    totCount += infos.count();
    totSize += combinedSize(infos);
}

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
    progressTimer.start();
    zeroCounters();
    QString lastPath;

    QQueue<QPair<QString, int>> dirQ;
    dirQ.enqueue({ startPath, 0 });

    while (!dirQ.empty() && !isStopped()) {
        processEvents();
        const auto [currPath, currDepth] = dirQ.dequeue();
        QFileInfoList dirInfos;
        getAllDirs(currPath, dirInfos);
        for (const auto& dir : dirInfos) {
            lastPath = currPath;
            //processEvents();
            if (stopped) {
                reportProgress(currPath, true);
                emit scanCancelled();
                return;
            }
            if (maxDepth < 0 || currDepth < maxDepth) {
                dirQ.enqueue({ dir.absoluteFilePath(), currDepth + 1 });
            }
            reportProgress(currPath);
        }
        updateTotals(currPath);

        QFileInfoList infos;
        getFileInfos(currPath, infos);
        for (const auto& info : infos) {
            processEvents();
            if (stopped) {
                reportProgress(currPath, true);
                emit scanCancelled();
                return;
            }
            if (appendOrExcludeItem(currPath, info)) {
                emit itemFound(info.absoluteFilePath(), info);
            }
            else {
                reportProgress(currPath);
            }
        }
    }
    reportProgress(lastPath, true);
    emit scanComplete();
    stopped = true;
}

std::pair<quint64, quint64> FolderScanner::deepCountSize(const QString& startPath)
{
    stopped = false;
    progressTimer.start();
    QQueue<QString> dirQ;
    dirQ.enqueue(startPath);
    zeroCounters();
    quint64 count = 0;
    quint64 size = 0;

    while (!dirQ.empty() && !stopped) {
        processEvents();
        const auto currPath = dirQ.dequeue();
        QFileInfoList dirInfos;
        getAllDirs(currPath, dirInfos);
        for (const auto& dir : dirInfos) {
            //processEvents();
            if (stopped) {
                return { count, size };
            }
            dirQ.enqueue(dir.absoluteFilePath());
            reportProgress(dir.absoluteFilePath());
        }

        QFileInfoList infos;
        getAllItems(currPath, infos);
        for (const auto& info : infos) {
            processEvents();
            if (stopped) {
                return { count, size };
            }
            ++count;
            size += getItemSize(info);
            foundCount = count;
            foundSize = size;
            reportProgress(info.absoluteFilePath());
            //emit itemSized(info.absoluteFilePath(), info);
        }
    }
    stopped = true;
    return { count, size };
}

void FolderScanner::deepRemove(const IntQStringMap& rowPathMap)
{
    stopped = false;
    progressTimer.start();
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
                const auto size = getItemSize(info);
                const auto rmok = file.remove();
                if (rmok) {
                    ++nbrDeleted;
                    emit itemRemoved(rowPath.first, 1, size, nbrDeleted);
                }
            }
            else {
                // RM DIR
                QDir dir(path);
                const auto [count, size] = deepCountSize(path);
                //processEvents();
                const auto rmok = dir.removeRecursively();
                processEvents();
                if (rmok) {
                    ++nbrDeleted;
                    emit itemRemoved(rowPath.first, count, size, nbrDeleted);
                }
            }
        }
        catch (...) { Q_ASSERT(false); } // tell the user?
    }
    stopped = true;
}

void FolderScanner::zeroCounters()
{
    foundCount = 0;
    foundSize = 0;
    totCount = 0;
    totSize = 0;
}

//void FolderScanner::incrCounters(quint64 fsize /*= 0*/)
//{
//    foundCount++;
//    foundSize += fsize;
//    totCount++;
//    totSize += fsize;
//}
}
