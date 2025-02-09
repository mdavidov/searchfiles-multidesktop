#pragma once

#include "common.h"
#include "scanparams.hpp"
#include <atomic>
#include <map>
#include <shared_mutex>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QString>
#include <QQueue>
#include <QTimer>
#include <QElapsedTimer>

namespace Devonline
{

class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent=nullptr);

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size, int nbrDeleted);
    void progressUpdate(quint64 foundCount, quint64 foundSize, quint64 totCount, quint64 totSize);
    void scanComplete();
    void scanCancelled();

public slots:
    void stop();
    void deepScan(const QString& startPath, const int maxDepth);
    std::pair<quint64, quint64> deepCountSize(const QString& startPath);
    void deepRemove(const IntQStringMap& itemList);

private slots:
    void reportProgress();

private:
    bool appendOrExcludeItem(const QString& dirPath, const QFileInfo& info);
    void getAllDirs(const QString& path, QFileInfoList& infos) const;
    void getFileInfos(const QString& path, QFileInfoList& infos) /*const*/;
    void getAllItems(const QString& path, QFileInfoList& infos) const;
    quint64 combinedSize(const QFileInfoList& items);
    void updateTotals(const QString& path);
    bool stringContainsAllWords(const QString& str, const QStringList& words);
    bool stringContainsAnyWord(const QString& str, const QStringList& words);
    bool fileContainsAllWordsChunked(const QString& path, const QStringList& words);
    bool fileContainsAnyWordChunked(const QString& path, const QStringList& words);

public:
    ScanParams params{};

private:
    mutable std::shared_mutex mutex;  // mutable allows modification in const methods
    bool stopped{ false };
    bool isStopped() const;

    QElapsedTimer eventsTimer;
    inline void processEvents();

    std::atomic<quint64> dirCount{0};
    std::atomic<quint64> totCount{0};
    std::atomic<quint64> totSize{0};
    std::atomic<quint64> foundSize{0};
    std::atomic<quint64> foundCount{0};
};

}
