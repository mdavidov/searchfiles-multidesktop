#pragma once

#undef QT_NO_CONTEXTMENU

#include "common.h"
#include "folderscanner.hpp"
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

namespace AmzQ {
    class FileRemover;
}
namespace Claude {
    class FileRemover;
}

namespace Devonline
{
class MainWindow;
class FolderScanner;


//class TableWidget : public QTableWidget
//{
//    Q_OBJECT
//public:
//    explicit TableWidget(int rows, int columns, QWidget* parent=nullptr) : QTableWidget(parent) {}
//protected:
//    void keyReleaseEvent(QKeyEvent* ev) override
//    {
//        // Move the implementation to the cpp file.
//        try
//        {
//            auto mwd = qobject_cast<MainWindow*>(parent());
//            Q_ASSERT(mwd);
//            if (!mwd) {
//                TableWidget::keyReleaseEvent(ev);
//                return;
//            }
//            if (ev->key() == Qt::Key_Delete) {
//                mwd->deleteBtnClicked();
//                ev->accept();
//            }
//            else if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
//                mwd->findBtnClicked();
//                ev->accept();
//            }
//            else if (ev->key() == Qt::Key_Escape) {
//                mwd->cancelBtnClicked();
//                ev->accept();
//            }
//            else
//                TableWidget::keyReleaseEvent(ev);
//        }
//        catch (...) { Q_ASSERT(false); }
//    }
//};


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
        enabled ?
            table->setSelectionMode(QAbstractItemView::ExtendedSelection) :
            table->setSelectionMode(QAbstractItemView::NoSelection);
        table->viewport()->setUpdatesEnabled(enabled);
        table->blockSignals(!enabled);
    }
private:
    QTableWidget* _table;
};


class TableWidgetItem : public QTableWidgetItem
{
public:
    TableWidgetItem() : QTableWidgetItem() {}
    explicit TableWidgetItem(const QString & itemText) : QTableWidgetItem(itemText) {}
private:
    bool operator<(const QTableWidgetItem & other) const override {
        return text().toLower() < other.text().toLower(); // performance problem?
    }
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& dirPath, QWidget* parent = nullptr);
    ~MainWindow();

    void SetDirPath( const QString & dirPath);
    Devonline::Op::Type GetOp() const { return _opType; }
    void Clear();

public slots:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemSized(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size);
    void progressUpdate(const QString& path, quint64 foundCount, quint64 foundSize, quint64 totCount, quint64 totSize);

    void findBtnClicked();
    void deleteBtnClicked();
    void shredBtnClicked();
    void cancelBtnClicked();

protected:
    void keyReleaseEvent(QKeyEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    void stopThreads();

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
    std::unique_ptr<QThread> scanThread;
    std::unique_ptr<FolderScanner> scanner;
    std::unique_ptr<AmzQ::FileRemover> removerAmzQ_;
    std::unique_ptr<Claude::FileRemover> removerClaude_;

    // We need to remove rows in decreasing order of row indices,
    // so we use a map sorted in descending order.
    // Boolean value is the result of file/folder removal.
    std::map<int, bool, std::greater<int>> rowsToRemove_;
    void removeRows();
    void removalProgress(int row, const QString& path, uint64_t size, bool rmOk);
    void removalComplete(bool success);
    QElapsedTimer progressTimer;

    QElapsedTimer eventsTimer;
    inline void processEvents();

    bool isHidden(const QFileInfo& finfo) const;
    QString FsItemType(const QFileInfo& finfo) const;

    QPushButton *createButton(const QString &text, const char *member);
    QComboBox * createComboBoxFSys(const QString & text, bool setCompleter = false);
    QComboBox * createComboBoxText();
    void createFilesTable();
    void appendItemToTable(const QString filePath, const QFileInfo & finfo);

    void deepScanFolderOnThread(const QString& startPath, const int maxDepth);
    void scanThreadFinished();
    void deepRemoveFilesOnThread_AmzQ(const IntQStringMap& paths);
    void deepRemoveFilesOnThread_Claude(const IntQStringMap& rowPathMap);

    void flushItemBuffer();

    bool findFilesPrep();
    void setStopped(bool stopped);
    void setFilesFoundLabel(const QString& prefix);

    void showMoreOptions(bool show);

    static void modifyFont(QWidget * widget, qreal ptSzDelta, bool bold, bool italic, bool underline);
    QFileSystemModel * newFileSystemModel(QCompleter* completer, const QString & currentDir);
    void setAllTips(QWidget * widget, const QString & text);

    void createSubDirLayout();
    void createItemTypeCheckLayout();
    void createNavigLayout();
    void createExclLayout();
    void createMainLayout();
    void createContextMenu();

    void getSelectedItems( IntQStringMap& itemList);

private:
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
    qint64 _dirCount;
    qint64 _totCount;
    quint64 _totSize;
    quint64 _foundSize;
    quint64 _foundCount;
    Devonline::Op::Type _opType;
    bool _stopped;
};

}
