#include "folderscanner.hpp"
#include "scanparams.hpp"
#include <shared_mutex>
#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include <QTimer>

namespace Devonline
{
FolderScanner::FolderScanner(QObject* parent)
: QObject(parent), progressTimer(this), prevElapsed(0)
{
    connect(&progressTimer, &QTimer::timeout, this, QOverload<>::of(&FolderScanner::reportProgress));
    progressTimer.setInterval(333); // msec
    progressTimer.setSingleShot(false);
    eventsTimer.start();
}

void FolderScanner::reportProgress() {
    emit progressUpdate(foundCount, foundSize, totCount, totSize);
}

void FolderScanner::stop() {
    std::unique_lock<std::shared_mutex> lock(mutex);
    stopped = true;
}

bool FolderScanner::isStopped() const {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return stopped;
}

inline bool FolderScanner::timeToProcEvents()
{
    const auto elapsed = eventsTimer.elapsed();
    if ((elapsed - prevElapsed) >= 200) {  // msec
        prevElapsed = elapsed;
        return true;
    }
    else
        return false;
}

void FolderScanner::doDeepScan(const QString& startPath, const int maxDepth) {
    stopped = false;
    foundCount = 0;
    progressTimer.start();

    QQueue<QPair<QString, int>> dirQ;
    dirQ.enqueue({startPath, 0});

    while (!dirQ.empty() && !isStopped()) {
        const auto [currPath, currDepth] = dirQ.dequeue();
        ++dirCount;
        QFileInfoList fileInfos;
        getFileInfos(currPath, fileInfos);
        updateTotals(currPath);

        for (const auto& info : fileInfos) {
            if (stopped) {
                progressTimer.stop();
                emit scanCancelled();
                return;
            }
            if (timeToProcEvents()) {
                qApp->processEvents();
            }
            auto add = false;
            if (info.isDir() && !info.isSymLink() && (currDepth < maxDepth || maxDepth < 0)) {
                dirQ.enqueue({info.absoluteFilePath(), currDepth + 1});
                add = appendOrExcludeItem(currPath, info);
            }
            else {
                add = appendOrExcludeItem(currPath, info);
            }
            if (add) {
                emit itemFound(info.absoluteFilePath(), info);
            }
        }
    }
    progressTimer.stop();
    emit scanComplete();
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
            if (info.isSymLink())
                toAppend = params.inclSymlinks;
            else if (info.isDir())
                toAppend = params.inclFolders;
        }
        if (info.isFile() && params.inclFiles) {
            toAppend = params.searchWords.empty() ||
                fileContainsAllWordsChunked(filePath, params.searchWords);
        }
        if (toAppend) {
            foundCount++;
            foundSize += quint64(info.size());
        }
        return toAppend;
    }
    catch (...) { Q_ASSERT(false); return false; } // TODO tell the user
}

void FolderScanner::getFileInfos(const QString& currPath, QFileInfoList& fileInfos) const
{
    QDir currDir(currPath);
    currDir.setNameFilters(params.fileNameSubfilters);
    currDir.setFilter(params.itemTypeFilter);
    QDir::Filters filters = QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot;
    if (!params.exclHidden)
        filters |= QDir::Hidden;
    fileInfos = currDir.entryInfoList(filters);
}

quint64 FolderScanner::combinedSize(const QFileInfoList& items)
{
    quint64 csz = 0;
    for (const auto& item : items) {
        if (stopped) return csz;
        csz += quint64(item.size());
    }
    return csz;
}

void FolderScanner::updateTotals(const QString& currPath)
{
    const QDir unFilteredDir = QDir(currPath);
    static constexpr QDir::Filters unfilters = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
    const QFileInfoList unFilteredItems = unFilteredDir.entryInfoList(unfilters);
    totCount += unFilteredItems.count();
    totSize += combinedSize(unFilteredItems);
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
}
