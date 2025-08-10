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
#include "folderscanner.hpp"
#include <chrono>
#include <memory>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfoList>
#include <QStringList>
#include <QtWidgets>

#pragma region
QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QProgressDialog;
class QFileinfo;
class QFileSystemModel;
class QKeyEvent;
class QLineEdit;
QT_END_NAMESPACE
#pragma endregion

#pragma region
#if defined(Q_OS_WIN)
#define eCod_MIN_PATH_LEN 3
#else
#define eCod_MIN_PATH_LEN 1
#endif
#pragma endregion

namespace Frv2 {
    class FileRemover;
}
namespace Frv3 {
    class FileRemover;
}

namespace Devonline
{
class MainWindow;
class FolderScanner;


/// @brief Blocks updates to the QTableWidget table in the constructor,
/// unblocks them in destructor.
class UpdateBlocker {
public:
    explicit UpdateBlocker(QTableWidget* table) : _table(table) {
        setAllUpdatesEnabled(_table, false);
    }
    ~UpdateBlocker() {
        setAllUpdatesEnabled(_table, true);
    }
    void setAllUpdatesEnabled(QTableWidget* table, bool enabled)
    {
        table->setSortingEnabled(false);
        table->setUpdatesEnabled(enabled);
        table->viewport()->setUpdatesEnabled(enabled);
        table->blockSignals(!enabled);
    }
private:
    QTableWidget* _table;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& dirPath, QWidget* parent);
    ~MainWindow();

    void SetDirPath( const QString & dirPath);
    Devonline::Op::Type GetOp() const { return _opType; }
    void Clear();

public slots:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemSized(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size, quint64 nbrDeleted);
    void progressUpdate(const QString& path, quint64 totCount, quint64 totSize);
    void removalComplete(bool success);

    void findBtnClicked();
    void deleteBtnClicked();
    void shredBtnClicked();
    void cancelBtnClicked();

protected:
    void keyReleaseEvent(QKeyEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    void stopAllThreads();

private slots:
    void scopeCheckClicked(int newCheckState);
    void goUpBtnClicked();
    void browseBtnClicked();
    void toggleExclClicked();
    void openFileOfItem(int row, int column);
    void itemSelectionChanged();
    void dirPathEditTextChanged(const QString & text);
    void completerTimeout();
    void openRunSlot();
    void openContainingFolderSlot();
    void copyPathSlot();
    void getSizeSlot();
    void propertiesSlot();
    void showContextMenu(const QPoint & point);
    void unlimSubDirDepthToggled(bool checked);
    void showAboutDialog();
    void showHelpDialog();

private:
    std::shared_ptr<QThread> scanThread;
    std::shared_ptr<FolderScanner> scanner;
    std::shared_ptr<Frv2::FileRemover> removerFrv2;
    std::shared_ptr<Frv3::FileRemover> removerFrv3;

    // We need to remove rows in decreasing order of row indices,
    // so we use a map sorted in descending (std::greater<int>) order.
    // Boolean value is the result of file/folder removal.
    std::map<int, bool, std::greater<int>> rowsToRemove_;
    void removeRows();
    void removalProgress(int row, const QString& path, uint64_t size, bool rmOk, uint64_t nbrDel);
    qint64 prevProgress{ 0 };
    QElapsedTimer progressTimer;

    qint64 prevEvents{ 0 };
    QElapsedTimer eventsTimer;
    inline void processEvents();

    std::shared_ptr<QElapsedTimer> runningTimer;
    std::chrono::steady_clock::time_point opStart;
    std::chrono::steady_clock::time_point opEnd;
    bool isHidden(const QFileInfo& finfo) const;
    QString FsItemType(bool isFile, bool isDir, bool isSymlink, bool isHidden) const;

    QPushButton* createButton(const QString& text, const char* member, QWidget* parent);
    QComboBox* createComboBoxFSys(const QString& text, bool setCompleter, QWidget* parent);
    QComboBox* createComboBoxText(QWidget* parent);
    void reCreateFilesTable();
    void appendItemToTable(const QString& filePath, const QFileInfo & finfo);

    void deepScanFolderOnThread(const QString& startPath, const int maxDepth);
    void scanThreadFinished();
    void deepRemoveLimitedOnThread(const IntQStringMap& itemList, const int maxDepth);
    void deepRemoveFilesOnThread_Frv2(const IntQStringMap& paths);
    void deepRemoveFilesOnThread_Frv3(const IntQStringMap& rowPathMap);
    void getSizeOnThread(const IntQStringMap& itemList);
    void getSizeImpl(const IntQStringMap& itemList);

    void flushItemBuffer();

    bool findFilesPrep();
    void setStopped(bool stopped);
    QString getElapsedTimeStr() const;
    void setFilesFoundLabel(const QString& prefix, bool appendCounts = true);

    void showMoreOptions(bool show);

    static void modifyFont(QWidget * widget, qreal ptSzDelta, bool bold, bool italic, bool underline);
    QFileSystemModel* newFileSystemModel(QCompleter* completer, const QString& currentDir);
    void setAllTips(QWidget * widget, const QString & text);

    void createSubDirLayout();
    void createItemTypeCheckLayout();
    void createNavigLayout();
    void createExclLayout();
    void createMainLayout();
    void createContextMenu();

    void getSelectedItems(IntQStringMap& itemList);

private:
    QLabel*     searchFolderLbl;
    QLineEdit*  namesLineEdit;
    QLineEdit*  wordsLineEdit;
    QComboBox*  dirComboBox;

    QLabel*     filesFoundLabel;

    QLabel*     itmTypeLbl;
    QCheckBox*  filesCheck;
    QCheckBox*  foldersCheck;
    QCheckBox*  symlinksCheck;
    QCheckBox*  matchCaseCheck;
    QHBoxLayout*itmTypeCheckLout;
    QHBoxLayout*wordsLout;

    QLabel *    maxSubDirDepthLbl;
    QRadioButton * unlimSubDirDepthBtn;
    QRadioButton * limSubDirDepthBtn;
    QLineEdit * maxSubDirDepthEdt;
    QHBoxLayout * subDirDepthLout;
    QHBoxLayout * navigLout;

    QToolButton * toggleExclBtn;
    QLineEdit * exclByFileNameCombo;
    QLineEdit * exclByFolderNameCombo;
    QLineEdit * exclFilesByTextCombo;
    QCheckBox * exclHiddenCheck;

    QToolButton *browseButton;
    QToolButton *goUpButton;
    QPushButton *findButton;
    QPushButton *deleteButton;
    QPushButton *shredButton;
    QPushButton *cancelButton;

    QTableWidget *filesTable;

    QList<QShortcut*> shortcuts;
    QAction* openRunAct;
    QAction* openContaingFolderAct;
    QAction* copyPathAct;
    QAction* getSizeAct;
    QAction* propertiesAct;
    QMenu* contextMenu;

    QWidget * centralWgt;

    QFileSystemModel * fileSystemModel;
    bool _ignoreDirPathChange; // TODO move to Completer (which will subclass QCompleter
    QTimer _completerTimer;    // TODO move to Completer (which will subclass QCompleter
    QElapsedTimer _editTextTimeDiff; // TODO move to Completer (which will subclass QCompleter

    QString _origDirPath;
    QString _fileNameFilter;
    QDir::Filters _itemTypeFilter;
    bool _matchCase;

    qint32 _maxSubDirDepth;
    bool _unlimSubDirDepth;

    QStringList _searchWords;
    QStringList _exclusionWords;
    QStringList _exclFilePatterns;
    QStringList _exclFolderPatterns;

    QFileInfoList _outFileInfos;

    quint64 _dirCount;
    quint64 _foundCount;
    quint64 _foundSize;
    quint64 _symlinkCount;
    quint64 _totCount;
    quint64 _totSize;
    quint64 _nbrDeleted;

    Devonline::Op::Type _opType;
    bool _stopped{ true };
    bool _removal{ false };
    bool _gettingSize{ false };
};

}

Q_DECLARE_METATYPE(Devonline::MainWindow)
