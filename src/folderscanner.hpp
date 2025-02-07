#pragma once

#include "scanparams.hpp"
#include <atomic>
#include <shared_mutex>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QQueue>
#include <QTimer>

namespace Devonline
{

template<class _Ty>
struct bigger {
    bool operator()(const _Ty& left, const _Ty& right) const {
        return (left > right);
    }
};
using Uint64StringMap = std::map<quint64, QString, bigger<quint64>>;

class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent=nullptr);

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void progressUpdate(quint64 foundCount, quint64 foundSize, quint64 totCount, quint64 totSize);
    void scanComplete();
    void scanCancelled();

public slots:
    void stop();
    void deepScan(const QString& startPath, const int maxDepth);
    std::pair<quint64, quint64> deepCountSize(const QString& startPath);
    void deepRemove(const Uint64StringMap& itemList);

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
    qint64 prevElapsed;
    inline bool timeToProcEvents();
    inline void processEvents();

    std::atomic<quint64> dirCount{0};
    std::atomic<quint64> totCount{0};
    std::atomic<quint64> totSize{0};
    std::atomic<quint64> foundSize{0};
    std::atomic<quint64> foundCount{0};
};

}
