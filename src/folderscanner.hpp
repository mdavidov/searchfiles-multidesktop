#pragma once

#include "scanparams.hpp"
#include <atomic>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QQueue>
#include <QTimer>

namespace Devonline
{

class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent = nullptr) : QObject(parent) {
        connect(&progressTimer, &QTimer::timeout, this, &FolderScanner::reportProgress);
        progressTimer.setInterval(200); // Report progress every 200ms
    }

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void progressUpdate(int itemsProcessed);
    void scanComplete();
    void scanCancelled();

public slots:
    void doDeepScan(const QString& startPath, const int maxDepth);

    void stop() {
        stopped = true;  // stopped is atomic
    }

private slots:
    void reportProgress() {
        emit progressUpdate(foundCount);
    }

private:
    bool appendOrExcludeItem(const QString& dirPath, const QFileInfo& info);
    void getFileInfos(const QString& path, QFileInfoList& infos) const;
    quint64 combinedSize(const QFileInfoList& items);
    void updateTotals(const QString& path);
    bool stringContainsAllWords(const QString& str, const QStringList& words);
    bool stringContainsAnyWord(const QString& str, const QStringList& words);
    bool fileContainsAllWordsChunked(const QString& path, const QStringList& words);
    bool fileContainsAnyWordChunked(const QString& path, const QStringList& words);

public:
    ScanParams params{};

private:
    QTimer progressTimer;
    std::atomic<bool> stopped{false};
    std::atomic<quint64> dirCount{0};
    std::atomic<quint64> totCount{0};
    std::atomic<quint64> totSize{0};
    std::atomic<quint64> foundSize{0};
    std::atomic<quint64> foundCount{0};
};

}
