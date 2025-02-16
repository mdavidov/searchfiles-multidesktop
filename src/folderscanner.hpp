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

using uint64pair = std::pair<uint64_t, uint64_t>;

namespace Devonline
{

class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent=nullptr);
    bool isStopped() const;
    ScanParams params{};
    quint64 getItemSize(const QFileInfo& info) const;
    quint64 combinedSize(const QFileInfoList& items);

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemSized(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size, int nbrDeleted);
    void progressUpdate(const QString& path, quint64 foundCount, quint64 foundSize, quint64 totCount, quint64 totSize);
    void scanComplete();
    void scanCancelled();

public slots:
    void stop();
    void deepScan(const QString& startPath, const int maxDepth);
    uint64pair deepCountSize(const QString& startPath);
    void deepRemove(const IntQStringMap& itemList);

private:
    void zeroCounters();
    bool appendOrExcludeItem(const QString& dirPath, const QFileInfo& info);
    void getAllDirs(const QString& path, QFileInfoList& infos) const;
    void getFileInfos(const QString& path, QFileInfoList& infos) /*const*/;
    void getAllItems(const QString& path, QFileInfoList& infos) const;
    void updateTotals(const QString& path);
    bool stringContainsAllWords(const QString& str, const QStringList& words);
    bool stringContainsAnyWord(const QString& str, const QStringList& words);
    bool fileContainsAllWordsChunked(const QString& path, const QStringList& words);
    bool fileContainsAnyWordChunked(const QString& path, const QStringList& words);

private:
    mutable std::shared_mutex mutex;  // mutable allows modification in const methods
    bool stopped{ false };

    qint64 prevEvents{ 0 };
    QElapsedTimer eventsTimer;
    void processEvents();

    qint64 prevProgress{ 0 };
    QElapsedTimer progressTimer;
    void reportProgress(const QString& path, bool doit = false);

    quint64 foundCount{0};
    quint64 foundSize{0};
    quint64 totCount{0};
    quint64 totSize{0};
};

}
