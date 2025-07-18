#pragma once
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

#include "common.h"
#include "scanparams.hpp"
#include "windows_symlink.hpp"
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
class TestFolderScanner;

namespace Devonline
{

bool isSymbolic(const QFileInfo& info);


class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent=nullptr);
    bool isStopped() const;
    ScanParams params{};
    quint64 combinedSize(const QFileInfoList& items);

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemSized(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size, quint64 nbrDeleted);
    void progressUpdate(const QString& path, quint64 totCount, quint64 totSize);
    void scanComplete();
    void scanCancelled();
    void removalComplete(bool success);
    void removalCancelled();

public slots:
    void stop();
    void deepScan(const QString& startPath, const int maxDepth);
    uint64pair deepCountSize(const QString& startPath);
    void deepRemove(const IntQStringMap& itemList);
    void deepRemoveLimited(const IntQStringMap& itemList, const int maxDepth);
    bool deepRemLimitedImpl(const QString& startPath, const int maxDepth, quint64& nbrDeleted);
    bool doRemoveOneFileOrDir(const QFileInfo& info, int row, quint64& nbrDeleted);
    bool rmEmptyDir(const QString& dirPath, int row, quint64& nbrDeleted);

public:
    void zeroCounters();
    bool appendOrExcludeItem(const QString& dirPath, const QFileInfo& info);
    void getAllDirs(const QString& path, QFileInfoList& infos);
    void getFileInfos(const QString& path, QFileInfoList& infos) /*const*/;
    void getAllItems(const QString& path, QFileInfoList& infos) const;
    // Not necessary: void updateTotals(const QString& path);
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

    quint64 dirCount{0};
    quint64 foundCount{0};
    quint64 foundSize{0};
    quint64 symlinkCount;
    quint64 totCount{0};
    quint64 totSize{0};
};

}

Q_DECLARE_METATYPE(Devonline::FolderScanner)
