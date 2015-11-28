/****************************************************************************
**
** Copyright (c) 2010 Milivoj (Mike) Davidov
** All rights reserved.
**
** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
** EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef _OVSK_FSOP_FINDINFILESDLG_H
#define _OVSK_FSOP_FINDINFILESDLG_H

#include "common.h"
#include <QMainWindow>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfoList>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QProgressDialog;
class QFileSystemModel;
class QKeyEvent;
class QLineEdit;
QT_END_NAMESPACE

namespace Overskys
{

class TableWidgetItem : public QTableWidgetItem
{
public:
    TableWidgetItem() : QTableWidgetItem() {}
    explicit TableWidgetItem(const QString & itemText) : QTableWidgetItem(itemText) {}
private:
    virtual /*override*/ bool operator<(const QTableWidgetItem & other) const {
        return text().toLower() < other.text().toLower(); // performance!
    }
};


class FindInFilesDlg : public QMainWindow
{
    Q_OBJECT

public:
    FindInFilesDlg(  const QString & dirPath, QWidget *parent = 0);
    void SetDirPath( const QString & dirPath);

    void GetFilePaths( QStringList & filePaths) const { filePaths = _outFiles; }
    void GetFileInfos( QFileInfoList & fileInfos) const;
    Overskys::Op::Type GetOp() const { return _opType; }
    void Clear();

protected:
    virtual void keyReleaseEvent( QKeyEvent* ev);

private slots:
    void scopeCheckClicked(int newCheckState);
    //void dlgFinished(int result);
    void goUpBtnClicked();
    void browseBtnClicked();
    void toggleExclClicked();
    void findBtnClicked();
    void deleteBtnClicked();
    void shredBtnClicked();
    void cancelBtnClicked();
    void openFileOfItem(int row, int column);
    void itemSelectionChanged();
    void dirPathEditTextChanged(const QString & text);
    void completerTimeout();
    void openRunSlot();
    void copyPathSlot();
    void propertiesSlot();
    void showContextMenu(const QPoint & point);
    void unlimSubDirDepthToggled(bool checked);

private:
    QStringList findTextInFiles(const QStringList &files, const QString &text);
    void showFiles(const QStringList &files);
    QPushButton *createButton(const QString &text, const char *member);
    QComboBox * createComboBoxFSys(const QString & text, bool setCompleter = false);
    QComboBox * createComboBoxText();
    void createFilesTable();
    void appendFileToTable(const QString filePath, const QFileInfo & fileInfo);

    void findFilesRecursive( const QString & dirPath, qint32 subDirDepth);
    bool findItem(const QString & dirPath, const QFileInfo& fileInfo);
    inline bool isTimeToReport();
    void setStopped(bool stopped);
    void setFilesFoundLabel(const QString & prefix = QString());
    quint64 combinedSize(const QFileInfoList& items);

    QStringList GetSimpleNamePatterns( const QString & rawNamePatters) const;
    bool StringContainsAnyWord( const QString & theString, const QStringList & wordList) const;

    bool fileContainsAllWords(const QString & filePath, const QStringList & wordList);
    bool fileContainsWord(    QFile & file,             const QString & word);
    bool fileContainsAnyWord( const QString & filePath, const QStringList & wordList);
    bool fileContainsAnyWord( QFile & file,             const QStringList & wordList);

    void showMoreOptions(bool show);

    static void modifyFont(QWidget * widget, qreal ptSzDelta, bool bold, bool italic, bool underline);
    QFileSystemModel * newFileSystemModel(QCompleter * completer, const QString & currentDir);
    void setAllTips(QWidget * widget, const QString & text);

    void createSubDirLayout();
    void createItemTypeCheckLayout();
    void createNavigLayout();
    void createExclLayout();
    void createMainLayout();

    void createRightClickMenu();

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

    QList<QShortcut *> shortcuts;
    QAction * openRunAct;
    QAction * copyPathAct;
    QAction * propertiesAct;
    QMenu * rightClickMenu;

    QWidget * centralWgt;

    QFileSystemModel * fileSystemModel;
    bool _ignoreDirPathChange; // TODO move to Completer (which will subclass QCompleter
    QTimer _completerTimer;    // TODO move to Completer (which will subclass QCompleter
    QElapsedTimer _editTextTimeDiff; // TODO move to Completer (which will subclass QCompleter
    QString _origDirPath;
    QString _fileNameFilter;
    QStringList _fileNameSubfilters;
    QDir::Filters _itemTypeFilter;
    bool _matchCase;

    qint32 _maxSubDirDepth;
    bool _unlimSubDirDepth;

    QStringList _searchWords;
    QStringList _exclusionWords;
    QStringList _exclFilePatterns;
    QStringList _exclFolderPatterns;

    QStringList _outFiles;
    QFileInfoList _outFileInfos;
    qint64 _dirCount;
    qint64 _totItemCount;
    quint64 _totItemsSize;
    quint64 _foundItemsSize;
    QElapsedTimer _elTimer;
    qint64 _prevEl;
    Overskys::Op::Type _opType;
    bool _stopped;
};

}

#endif
