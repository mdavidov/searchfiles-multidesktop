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

#undef QT_NO_CONTEXTMENU

#if defined(Q_OS_WIN)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#undef ERROR
#undef min
#undef max
#endif

#include "precompiled.h"
#include "fileremover-amzq.hpp"
#include "fileremover-claude.hpp"
#include "foldersearch.hpp"
#include "scanparams.hpp"
#include "aboutdialog.h"
#include <algorithm>
#include "helpdialog.h"
#include "config.h"
#include "keypress.hpp"
#include <memory>
#include "util.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QThread>
#include <QProcess>
#include <QDesktopServices>
#include <QtGui>
#include <QtWidgets>
#include <QStandardPaths>
#include <QtCore/QRegularExpression>

#if defined(Q_OS_MAC)
extern "C" void showFileProperties(const char* filePath);
#endif

namespace Devonline
{
const QString findText( "&Search");
const QString stopText( "S&top");

const int N_COL = 7;
const int RELPATH_COL_IDX = 0;

static int BATCH_SIZE = 100;
QVector<QVector<QTableWidgetItem*>> itemBuffer;

#if defined(Q_OS_WIN)
    #define eCod_MIN_PATH_LEN 3
#else
    #define eCod_MIN_PATH_LEN 1
#endif

bool MainWindow ::isHidden(const QFileInfo& finfo) const {
    return  finfo.isHidden() || finfo.fileName().startsWith('.');
}

MainWindow::~MainWindow()
{
    // This will also delete the actions that were added to the context menu
    delete contextMenu;
}

MainWindow::MainWindow( const QString & /*dirPath*/, QWidget * parent)
    : QMainWindow(parent)
    , _ignoreDirPathChange(false)
    , _origDirPath( QDir::toNativeSeparators( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory)))
    , _maxSubDirDepth(0)
    , _unlimSubDirDepth(true)
    , _stopped(true)
{
    setWindowTitle(QString(OvSk_FsOp_APP_NAME_TXT) + " " + OvSk_FsOp_APP_VERSION_STR);  // + " " + OvSk_FsOp_APP_BUILD_NBR_STR);
    const auto savedPath = Cfg::St().value(Cfg::origDirPathKey).toString();
    if (!savedPath.isEmpty())
        _origDirPath = savedPath;

    createSubDirLayout();
    createItemTypeCheckLayout();
    createNavigLayout();
    createExclLayout();
    createFilesTable();
    createMainLayout();

    showMoreOptions(true);
    itemSelectionChanged();
    SetDirPath(_origDirPath);

    _completerTimer.setSingleShot(true);
    bool c = connect(&_completerTimer, SIGNAL(timeout()), this, SLOT(completerTimeout())); Q_ASSERT(c);
    _editTextTimeDiff.restart();

    c = connect(filesCheck,    SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;
    c = connect(foldersCheck,  SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;
    c = connect(symlinksCheck, SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;

    createContextMenu();

    // Menu bar
    {
       QMenu* helpMenu = menuBar()->addMenu("&Help");
       QAction* aboutAction = helpMenu->addAction("&About");
       QAction* helpAction = helpMenu->addAction("&Help");
       connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
       connect(helpAction,  &QAction::triggered, this, &MainWindow::showHelpDialog);
    }

    findButton->setDefault(true);
    findButton->setFocus();
    setStopped(true);

    auto filter = new KeyPressEventFilter(this);
    this->installEventFilter(filter);
    connect(filter, &KeyPressEventFilter::enterKeyPressed,
            this,   &MainWindow::onEnterKeyPressed);
}

void MainWindow::onEnterKeyPressed()
{
    findBtnClicked();
}

void MainWindow::createSubDirLayout()
{
    maxSubDirDepthLbl = new QLabel(tr("Max subfolder depth: "));
    setAllTips(maxSubDirDepthLbl, tr("Specify unlimited or maximum sub-folder depth."));
    unlimSubDirDepthBtn = new QRadioButton(tr("Unlimited "));
    unlimSubDirDepthBtn->setChecked(true);
    limSubDirDepthBtn = new QRadioButton(tr("Limited to: "));
    maxSubDirDepthEdt = new QLineEdit();
    maxSubDirDepthEdt->setEnabled(false);
    maxSubDirDepthEdt->setFixedWidth(60);
    QIntValidator* intValidator = new QIntValidator();
    intValidator->setBottom(0);
    maxSubDirDepthEdt->setValidator(intValidator);
    QButtonGroup* subDirDepthGrp = new QButtonGroup(this);
    subDirDepthGrp->setExclusive(true);
    subDirDepthGrp->addButton(unlimSubDirDepthBtn, 1);
    subDirDepthGrp->addButton(limSubDirDepthBtn,   1);

    subDirDepthLout = new QHBoxLayout();
    subDirDepthLout->addWidget( maxSubDirDepthLbl);
    subDirDepthLout->addWidget( unlimSubDirDepthBtn);
    subDirDepthLout->addWidget( limSubDirDepthBtn);
    subDirDepthLout->addWidget( maxSubDirDepthEdt);
    subDirDepthLout->addStretch();
    bool c = connect(unlimSubDirDepthBtn, &QRadioButton::toggled, this, &MainWindow::unlimSubDirDepthToggled); Q_ASSERT(c); (void)c;
}

void MainWindow::createItemTypeCheckLayout()
{
    itmTypeLbl = new QLabel(tr("Types:"));
    setAllTips(itmTypeLbl, eCod_SEARCH_BY_TYPE_TIP);
    filesCheck    = new QCheckBox(tr("&Files"));
    foldersCheck  = new QCheckBox(tr("Folders"));
    symlinksCheck = new QCheckBox(OvSk_FsOp_SYMLINKS_TXT);
    filesCheck->setChecked( true);
    foldersCheck->setChecked( true);
    symlinksCheck->setChecked( true);

    itmTypeCheckLout = new QHBoxLayout();
    itmTypeCheckLout->addWidget( filesCheck);
    itmTypeCheckLout->addWidget( foldersCheck);
    itmTypeCheckLout->addWidget( symlinksCheck);
    itmTypeCheckLout->addSpacing(40);
    itmTypeCheckLout->addLayout( subDirDepthLout);
    itmTypeCheckLout->addStretch();
}

void MainWindow::createNavigLayout()
{
    browseButton = new QToolButton();
    browseButton->setText(tr("Browse..."));
    setAllTips(browseButton, eCod_BROWSE_FOLDERS_TIP);
    bool c = connect(browseButton, SIGNAL(clicked()), this, SLOT(browseBtnClicked())); Q_ASSERT(c);

    goUpButton = new QToolButton();
    goUpButton->setText(tr("^"));
    setAllTips(goUpButton, eCod_BROWSE_GO_UP_TIP);
    c = connect(goUpButton, SIGNAL(clicked()), this, SLOT(goUpBtnClicked())); Q_ASSERT(c);

    navigLout = new QHBoxLayout();
    navigLout->addWidget( browseButton);
    navigLout->addWidget( goUpButton);
    navigLout->addStretch();

    findButton   = createButton(findText,         SLOT(findBtnClicked()));
    deleteButton = createButton(tr("&Delete"),    SLOT(deleteBtnClicked()));
    shredButton  = createButton(tr("S&hred"),     SLOT(shredBtnClicked()));
    cancelButton = createButton(tr("S&top"),      SLOT(cancelBtnClicked()));
    modifyFont(findButton, +1.0, true, false, false);

    dirComboBox = createComboBoxFSys( _origDirPath, true);
    setAllTips(dirComboBox, OvSk_FsOp_TOP_DIR_TIP);
    modifyFont(dirComboBox, +0.0, true, false, false);
    c = connect( dirComboBox,    SIGNAL(editTextChanged(const QString &)),
                 this,      SLOT(dirPathEditTextChanged(const QString &))); Q_ASSERT(c);

    wordsLineEdit = new QLineEdit(); //createComboBoxText();
    wordsLineEdit->setPlaceholderText("Search words");
    setAllTips(wordsLineEdit, OvSk_FsOp_CONTAINING_TEXT_TIP);
    modifyFont(wordsLineEdit, +0.0, true, false, false);
    matchCaseCheck = new QCheckBox(tr("&Match case"));
    setAllTips(matchCaseCheck, "Match or ignore the case of letters in search words. Does not affect file/folder names. ");

    wordsLout = new QHBoxLayout();
    wordsLout->addWidget(wordsLineEdit);
    wordsLout->addWidget(matchCaseCheck);

    namesLineEdit = new QLineEdit(); //createComboBoxText();
    namesLineEdit->setPlaceholderText("File/Folder names");
    setAllTips(namesLineEdit, OvSk_FsOp_NAME_FILTERS_TIP);
    modifyFont(namesLineEdit, +0.0, true, false, false);
}

void MainWindow::createExclLayout()
{
    toggleExclBtn = new QToolButton();
    toggleExclBtn->setText(tr("-"));
    setAllTips(toggleExclBtn, eCod_SHOW_EXCL_OPTS_TIP);
    bool c = connect(toggleExclBtn, SIGNAL(clicked()), this, SLOT(toggleExclClicked())); Q_ASSERT(c); (void)c;

    exclFilesByTextCombo = new QLineEdit(); //createComboBoxText();
    exclFilesByTextCombo->setPlaceholderText("Exclude files with words");
    setAllTips(exclFilesByTextCombo, eCod_EXCL_FILES_BY_CONTENT_TIP);
    modifyFont(exclFilesByTextCombo, +0.0, false, false, false);

    exclByFileNameCombo = new QLineEdit(); //createComboBoxText();
    exclByFileNameCombo->setPlaceholderText("Exclude by partial file names");
    setAllTips(exclByFileNameCombo, eCod_EXCL_FILES_BY_NAME_TIP);
    modifyFont(exclByFileNameCombo, +0.0, false, false, false);

    exclByFolderNameCombo = new QLineEdit(); //createComboBoxText();
    exclByFolderNameCombo->setPlaceholderText("Exclude by partial folder names");
    setAllTips(exclByFolderNameCombo, eCod_EXCL_FOLDERS_BY_NAME_TIP);
    modifyFont(exclByFolderNameCombo, +0.0, false, false, false);

    exclHiddenCheck = new QCheckBox();
    exclHiddenCheck->setChecked(false);
    exclHiddenCheck->setText("Exclude hidden");
    setAllTips(exclHiddenCheck, eCod_EXCL_HIDDEN_ITEMS);
    modifyFont(exclHiddenCheck, +0.0, false, false, false);
}

void MainWindow::createMainLayout()
{
    filesFoundLabel = new QLabel;
    modifyFont(filesFoundLabel, +0.0, true, false, false);

    auto sfLabel = new QLabel("Search folder {SF}:");
    sfLabel->setToolTip("Folder to deeply search, a.k.a. {SF}");
    modifyFont(sfLabel, +0.0, true, false, false);

    auto dirComboLayout = new QHBoxLayout;
    dirComboLayout->addWidget(sfLabel);
    dirComboLayout->addWidget(dirComboBox);
    dirComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    //buttonsLayout->addWidget(shredButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(findButton);

    int gridRowIdx = 0;
    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addLayout(navigLout,        gridRowIdx, 0);
    ++gridRowIdx;

    mainLayout->addLayout(dirComboLayout,   gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addLayout(wordsLout,        gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addWidget(namesLineEdit,    gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addLayout(itmTypeCheckLout, gridRowIdx, 0);
    ++gridRowIdx;

    mainLayout->addWidget(toggleExclBtn,    gridRowIdx, 0);
    ++gridRowIdx;

    mainLayout->addWidget(exclFilesByTextCombo, gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addWidget(exclByFileNameCombo,  gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addWidget(exclByFolderNameCombo,gridRowIdx, 0, 1, 3);
    ++gridRowIdx;

    mainLayout->addWidget(exclHiddenCheck,      gridRowIdx, 0);
    ++gridRowIdx;

    mainLayout->addWidget(filesTable,           gridRowIdx, 0, 1, 4);
    ++gridRowIdx;
    mainLayout->addWidget(filesFoundLabel,      gridRowIdx, 0, 1, 4);
    ++gridRowIdx;

    mainLayout->addLayout(buttonsLayout,        gridRowIdx, 0, 1, 4);
    ++gridRowIdx;

    centralWgt = new QWidget();
    setCentralWidget(centralWgt);
    centralWgt->setLayout(mainLayout);
    resize(1000, 625);
}

void MainWindow::modifyFont(QWidget * widget, qreal ptSzDelta, bool bold, bool italic, bool underline)
{
    QFont font(widget->font());
    font.setPointSizeF(font.pointSizeF() + ptSzDelta);
    font.setBold(bold);
    font.setItalic(italic);
    font.setUnderline(underline);
    widget->setFont(font);
}

void MainWindow::setAllTips(QWidget * widget, const QString & text)
{
    widget->setToolTip(  text);
    widget->setStatusTip(text); // does not work in dialog based apps
    widget->setWhatsThis(text);
}

void MainWindow::scopeCheckClicked(int /*newCheckState*/)
{
    const bool filesChecked = (filesCheck->checkState() == Qt::Checked);
    wordsLineEdit->setEnabled(filesChecked);
    matchCaseCheck->setEnabled(filesChecked);
    exclByFileNameCombo->setEnabled(filesChecked);
    exclFilesByTextCombo->setEnabled(filesChecked);
}

void MainWindow::goUpBtnClicked()
{
    const QString dirPath = dirComboBox->currentText().trimmed();
    if (dirPath.isEmpty())
        return;
    QDir dir(dirPath);

    const bool ok = dir.cdUp();
    if (ok)
        SetDirPath( QDir::toNativeSeparators( dir.absolutePath()));
    else if (dirPath.length() <= eCod_MIN_PATH_LEN)
        SetDirPath( "");
}

void MainWindow::browseBtnClicked()
{
    const QString selDir = QFileDialog::getExistingDirectory(this, tr("Select folder"),
                            dirComboBox->currentText().trimmed());
    if (!selDir.isEmpty())
        SetDirPath(selDir);
}

void MainWindow::showMoreOptions(bool show)
{
    toggleExclBtn->setText(show ? "-" : "+");
    toggleExclBtn->setToolTip(show ? eCod_SHOW_EXCL_OPTS_TIP : eCod_HIDE_EXCL_OPTS_TIP);
    exclByFolderNameCombo->setVisible(show);
    exclByFileNameCombo->setVisible(show);
    exclFilesByTextCombo->setVisible(show);
    exclHiddenCheck->setVisible(show);
}

void MainWindow::toggleExclClicked()
{
    const bool show = toggleExclBtn->text().startsWith("+");
    const int deltaH = 100; // TODO calc the height of excl. widgets
    showMoreOptions(show);
    resize(width(), height() + (show ? +deltaH : -deltaH));
}

static void updateComboBox(QComboBox *comboBox)
{
    if (!comboBox->currentText().isEmpty() && comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

/////////////////////////////////////////////////

void MainWindow::deleteBtnClicked()
{
    try {
        _opType = Devonline::Op::deletePerm;
        if (filesTable->selectedItems().empty()) {
            #if !defined(Q_OS_MAC)
                QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
            #else
                qDebug() << OvSk_FsOp_SELECT_FOUNDFILES_TXT;
            #endif
            // return;
        }
        setStopped(false);
        processEvents();
        Uint64StringMap itemList;
        QStringList pathList;
        getSelectedItems(itemList, pathList);

        // REMOVE ITEMS
        //removeItems(itemList);
        deepRemoveFilesOnThread_Claude(pathList);
        // deepRemoveFilesOnThread_AmzQ(pathList);

        //emit filesTable->itemSelectionChanged();
        //processEvents();
        //setFilesFoundLabel(_stopped ? "INTERRUPTED. " : "COMPLETED. ");
        //setStopped(true);
    }
    catch (...) { Q_ASSERT(false); } // tell the user?
    setStopped(true);
}

void MainWindow::getSelectedItems(Uint64StringMap& itemList, QStringList& pathList)
{
    const QList<QTableWidgetItem *>  selectedItems = filesTable->selectedItems();
    foreach (const QTableWidgetItem * item, selectedItems)
    {
        if (filesTable->column( item) == RELPATH_COL_IDX)
        {
            if (_stopped)
                break;
            const QString path = item->data(Qt::UserRole).toString(); //item->text();
            const auto row = static_cast<quint64>(filesTable->row(item));
            Q_ASSERT(itemList.find(row) == itemList.end());
            if (itemList.find(row) == itemList.end()) {
                itemList.insert(std::pair<int, QString>(row, path));
                pathList.append(path);
            }
            processEvents();
        }
    }
}

void MainWindow::shredBtnClicked()
{
    try
    {
        _opType = Devonline::Op::shredPerm;
        setStopped(true);
        if (filesTable->selectedItems().empty()) {
            #if !defined(Q_OS_MAC)
                QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
            #else
                qDebug() << OvSk_FsOp_SELECT_FOUNDFILES_TXT;
            #endif
            // return;
        }

        // TODO: IMPLEMENT Shredding

        emit filesTable->itemSelectionChanged();
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::cancelBtnClicked()
{
    setStopped(true);
    scanner->stop();
    //reject();
}

void MainWindow::SetDirPath( const QString& dirPath)
{
    try
    {
        if (dirPath.isEmpty()) {
            dirComboBox->setCurrentText("");
            return;
        }
        _origDirPath = QDir::toNativeSeparators( dirPath);
        if (dirComboBox->findText(_origDirPath) == -1)
            dirComboBox->addItem(_origDirPath);
        dirComboBox->setCurrentIndex( dirComboBox->findText(_origDirPath));
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::Clear()
{
    try
    {
        filesTable->setRowCount(0);
        filesTable->hide();
        filesTable->show();
        filesFoundLabel->setText("");
        _dirCount = 0;
        _totCount = 0;
        _totSize = 0;
        _foundCount = 0;
        _foundSize = 0;
        setStopped(true);
        processEvents();
        eventsTimer.start();
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::setStopped(bool stopped)
{
  try
  {
    _stopped = stopped;

    findButton->setEnabled(     _stopped);
    deleteButton->setEnabled(   _stopped && filesTable->selectedItems().count() > 0);
    shredButton->setEnabled(    _stopped && filesTable->selectedItems().count() > 0);
    cancelButton->setEnabled(  !_stopped);

    namesLineEdit->setEnabled(  _stopped);

    dirComboBox->setEnabled(    _stopped);

    itmTypeLbl->setEnabled(     _stopped);
    filesCheck->setEnabled(     _stopped);
    foldersCheck->setEnabled(   _stopped);
    symlinksCheck->setEnabled(  _stopped);
    unlimSubDirDepthBtn->setEnabled(_stopped);
    limSubDirDepthBtn->setEnabled(  _stopped);
    maxSubDirDepthEdt->setEnabled(  _stopped && limSubDirDepthBtn->isChecked());
    goUpButton->setEnabled(     _stopped);
    browseButton->setEnabled(   _stopped);

    wordsLineEdit->setEnabled(  _stopped && filesCheck->isChecked());
    matchCaseCheck->setEnabled( _stopped && filesCheck->isChecked());

    exclByFolderNameCombo->setEnabled(_stopped);
    exclByFileNameCombo->setEnabled(_stopped);
    exclFilesByTextCombo->setEnabled(_stopped);
    exclHiddenCheck->setEnabled(_stopped);

    filesTable->horizontalHeader()->setEnabled( _stopped);
    filesTable->verticalHeader()->setEnabled(   _stopped);
  }
  catch (...) { Q_ASSERT(false); }
}

void MainWindow::setFilesFoundLabel(const QString& prefix)
{
    QString totItemsSizeStr;
    sizeToHumanReadable(_totSize, totItemsSizeStr);
    QString foundItemsSizeStr;
    sizeToHumanReadable(_foundSize, foundItemsSizeStr);
    const auto itemsTxt = filesTable->rowCount() == 1 ? "item" : "items";
    const auto foundLabelText =
        prefix
        + tr("Found %1 matching %2, %3 (searched %4 items, %5). ")
            .arg(_foundCount)  // (filesTable->rowCount())
            .arg(itemsTxt)
            .arg(foundItemsSizeStr)
            .arg(_totCount)
            .arg(totItemsSizeStr);
    filesFoundLabel->setText("");
    filesFoundLabel->setText(foundLabelText);
    qDebug() << foundLabelText;
    if (_foundCount != quint64(filesTable->rowCount())) {
        qDebug() << "WARNING: _foundCount" << _foundCount
                 << "!= filesTable->rowCount()" << filesTable->rowCount();
    }
}

bool MainWindow::findFilesPrep(FolderScanner* scanner_)
{
    scanner = scanner_;

    // SET QDir FILTERS
    _fileNameFilter = namesLineEdit->text();
    scanner->params.nameFilters = namesLineEdit->text().split(" ", Qt::SkipEmptyParts);
    updateComboBox( dirComboBox);

    _itemTypeFilter = QDir::Filters(QDir::Drives | QDir::System | QDir::NoDotAndDotDot);
    if (filesCheck->isChecked())
        _itemTypeFilter |= QDir::Files;
    if (foldersCheck->isChecked() && _searchWords.isEmpty())
        _itemTypeFilter |= QDir::Dirs;
    if (!symlinksCheck->isChecked() || !_searchWords.isEmpty())
        _itemTypeFilter |= QDir::NoSymLinks;
    if (!exclHiddenCheck->isChecked())
        _itemTypeFilter |= QDir::Hidden;
    scanner->params.itemTypeFilter = _itemTypeFilter;

    scanner->params.inclFiles = filesCheck->isChecked() || symlinksCheck->isChecked();
    scanner->params.inclFolders = foldersCheck->isChecked() || symlinksCheck->isChecked();
    scanner->params.inclSymlinks = symlinksCheck->isChecked();
    scanner->params.exclHidden = exclHiddenCheck->isChecked();

    _matchCase = matchCaseCheck->isChecked();
    scanner->params.matchCase = _matchCase;

    const auto dirComboCurrent = dirComboBox->currentText().trimmed();
    if (dirComboCurrent.length() == 0) {
        qDebug() << "Select a folder to search please.";
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, QString("Select a folder to search please."));
        #else
            qDebug() << "Select a folder to search please.";
	    #endif
        return false;
    }
    _origDirPath = QDir::toNativeSeparators(dirComboCurrent);
    qDebug() << "_origDirPath" << _origDirPath;
    if (_origDirPath.startsWith("~")) {
        _origDirPath = (_origDirPath.length() > 1) ?
            QDir::homePath() + _origDirPath.sliced(1) :
            QDir::homePath();
        qDebug() << "_origDirPath" << _origDirPath;
    }
    QDir origDir = QDir(_origDirPath);
    if (_origDirPath.isEmpty() || !origDir.exists()) {
        const QString msg = OvSk_FsOp_DIR_NOT_EXISTS_TXT + _origDirPath;
        qDebug() << msg;
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, msg);
        #else
            qDebug() << msg;
	    #endif
        return false;
    }
    while (_origDirPath.length() > eCod_MIN_PATH_LEN && _origDirPath.endsWith( QDir::separator())) {
        _origDirPath.chop(1);
    }
    if (_origDirPath.length() == 2 && _origDirPath.endsWith(":")) {
		_origDirPath += QDir::separator();
    }
    scanner->params.origDirPath = _origDirPath;

    _searchWords        = wordsLineEdit->text().split(" ", Qt::SkipEmptyParts);
    _exclusionWords     = exclFilesByTextCombo->text().split(" ", Qt::SkipEmptyParts);
    _exclFilePatterns   = exclByFileNameCombo->text().split(" ", Qt::SkipEmptyParts);
    _exclFolderPatterns = exclByFolderNameCombo->text().split(" ", Qt::SkipEmptyParts);
    scanner->params.searchWords = _searchWords;
    scanner->params.exclusionWords = _exclusionWords;
    scanner->params.exclFilePatterns = _exclFilePatterns;
    scanner->params.exclFolderPatterns = _exclFolderPatterns;

    BATCH_SIZE = (_searchWords.isEmpty() && _exclusionWords.isEmpty()) ? 300 : 1;

    bool maxValid = false;
    _maxSubDirDepth =  maxSubDirDepthEdt->text().toInt(&maxValid);
    if (!maxValid)
        _maxSubDirDepth = 0;
    _unlimSubDirDepth = unlimSubDirDepthBtn->isChecked();
    return true;
}

void MainWindow::findBtnClicked()
{
  try
  {
    if (!_stopped) {
        scanner->stop();
        setStopped(true);
        setFilesFoundLabel(tr("INTERRUPTED. "));
        filesTable->sortByColumn(-1, Qt::AscendingOrder);
        filesTable->setSortingEnabled(true);
        return;
    }
    if (!filesCheck->isChecked() && !foldersCheck->isChecked() && !symlinksCheck->isChecked()) {
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_ITEM_TYPE_TXT);
        #else
            qDebug() << OvSk_FsOp_SELECT_ITEM_TYPE_TXT;
	    #endif
        // return;
    }
    filesTable->setSortingEnabled( false);
    Clear();
    setStopped(false);

    deepScanFolderOnThread(_origDirPath, _unlimSubDirDepth ? -1 : _maxSubDirDepth);
  }
  catch (...) { Q_ASSERT(false); } // tell the user?
}

inline void MainWindow::processEvents()
{
    const auto elapsed = eventsTimer.elapsed();
    // Like all static vars, local function statics are initialised only once,
    // first time the function is called.
    static qint64 prev = elapsed;
    const auto diff = elapsed - prev;
    if (diff >= 200) {  // msec
        // Static var prev keeps its value for the next call of this function.
        prev = elapsed;
        qApp->processEvents();
    }
}

QPushButton * MainWindow::createButton(const QString & text, const char *member)
{
    QPushButton *button = new QPushButton(text);
    const bool c = connect(button, SIGNAL(clicked()), this, member); Q_ASSERT(c); (void)c;
    return button;
}

#if defined(Q_OS_WIN)
    #define eCod_TOP_ROOT_PATH "C:\\"
#else
    #define eCod_TOP_ROOT_PATH "/"
#endif

void MainWindow::dirPathEditTextChanged(const QString& newText)
{
    try
    {
        if (_ignoreDirPathChange) {
            _ignoreDirPathChange = false;
            return;
        }
        if (newText.isEmpty())
            return;
        const qint64 timeDiff = _editTextTimeDiff.elapsed();

        if (newText.endsWith(QDir::separator())) {
            fileSystemModel->setRootPath(newText);
            dirComboBox->completer()->setCompletionPrefix(newText);
            #if defined(Q_OS_WIN)
                if (newText.length() == 1) {
                    _ignoreDirPathChange = true;
                    fileSystemModel->setRootPath(eCod_TOP_ROOT_PATH);
                    dirComboBox->completer()->setCompletionPrefix(eCod_TOP_ROOT_PATH);
                    dirComboBox->setCurrentText( eCod_TOP_ROOT_PATH);
                }
            #endif
            _completerTimer.start(33);
        }
        else if (newText.isEmpty()) {
            if (timeDiff < 40 || timeDiff > 120) {
                fileSystemModel->setRootPath("");
                dirComboBox->completer()->setCompletionPrefix("");
                _completerTimer.start(33);
            }
        }
        else { // newText is not empty
            if (dirComboBox->completer()->completionPrefix().isEmpty() && timeDiff < 30) {
                dirComboBox->completer()->setCompletionPrefix(newText);
            }
        }
        Cfg::St().setValue( Cfg::origDirPathKey,  QDir::toNativeSeparators(newText));
        _editTextTimeDiff.restart();
    }
    catch(...) { Q_ASSERT(false); }
}

void MainWindow::completerTimeout()
{
    try
    {
        if (dirComboBox->completer())
            dirComboBox->completer()->complete(); // shows the popup if the completion count > 0
    }
    catch(...) { Q_ASSERT(false); }
}

QFileSystemModel * MainWindow::newFileSystemModel(QCompleter * completer, const QString & currentDir)
{
    fileSystemModel = new QFileSystemModel(completer);
    fileSystemModel->setReadOnly( true);
    fileSystemModel->setResolveSymlinks( false);
    fileSystemModel->setFilter( QDir::Dirs | QDir::Drives  | QDir::Hidden  | QDir::System | QDir::NoDotAndDotDot);
    fileSystemModel->setRootPath(currentDir);
    //fileSystemModel->sort(0, Qt::AscendingOrder);
    return fileSystemModel;
}

QComboBox * MainWindow::createComboBoxFSys(const QString & text, bool setCompleter)
{
    QCompleter * completer = NULL;
    if (setCompleter)
    {
        completer = new QCompleter();
        #if defined(Q_OS_WIN)
        {
            completer->setCaseSensitivity( Qt::CaseInsensitive);
            completer->setModelSorting( QCompleter::CaseSensitivelySortedModel);
        }
        #else
        {
            completer->setCaseSensitivity( Qt::CaseSensitive);
            completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel);
        }
        #endif
        completer->setCompletionMode( QCompleter::PopupCompletion);
        completer->setMaxVisibleItems( 16);
        completer->setWrapAround( false);
    }

    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    if (completer)
    {
        completer->setParent(comboBox);
        completer->setModel( newFileSystemModel(completer, text));
        comboBox->setCompleter( completer);
    }

    return comboBox;
}

QComboBox* MainWindow::createComboBoxText()
{
    QCompleter* completer = new QCompleter();
    completer->setCaseSensitivity( Qt::CaseSensitive);
    completer->setModelSorting( QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode( QCompleter::InlineCompletion);
    completer->setMaxVisibleItems( 16);
    completer->setWrapAround( false);

    QComboBox* comboBox = new QComboBox;
    comboBox->setEditable( true);
    comboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred);

    completer->setParent( comboBox);
    //completer->setModel( ? );
    comboBox->setCompleter( completer);

    return comboBox;
}

QString MainWindow::FsItemType(const QFileInfo & finfo) const
{
    QString fsType;
    if (finfo.isSymLink()) {
        if (finfo.isDir())
            fsType = OvSk_FsOp_SYMLINK_TXT " to folder";
        else if (finfo.isFile())
            fsType = OvSk_FsOp_SYMLINK_TXT " to file";
        else
            fsType = OvSk_FsOp_SYMLINK_TXT;
    }
    else if (finfo.isDir())
        fsType = "Folder";
    else if (finfo.isFile())
        fsType = "File";
    else
        fsType = "";

    if (isHidden(finfo))
        fsType += " - hidden";

    return fsType;
}

void MainWindow::appendItemToTable(const QString filePath, const QFileInfo& finfo)
{
  try
  {
    processEvents();
    auto fileNameItem = new TableWidgetItem;
    auto fileName = finfo.fileName();
    if (finfo.isSymLink() && !finfo.symLinkTarget().isEmpty())
        fileName += " -> " + finfo.symLinkTarget();
    else if (finfo.isDir())
        fileName += QDir::separator();
    fileNameItem->setData(Qt::DisplayRole, QDir::toNativeSeparators(fileName));
    fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
    auto absFilePath = QDir::toNativeSeparators(finfo.absoluteFilePath());
    if (finfo.isDir())
        absFilePath += QDir::separator();
    fileNameItem->setData(Qt::ToolTipRole, QVariant(absFilePath));

    const QString fpath = QDir::toNativeSeparators( finfo.path());
    const auto dlen = QDir::toNativeSeparators( _origDirPath).length();
    QString relPath = (fpath.length() <= dlen) ? "" : fpath.right( fpath.length() - dlen);
    if (relPath.startsWith( QDir::separator()))
        relPath = relPath.right( relPath.length() - 1);
    relPath = "{SF}/" + relPath + (relPath.length() > 0 ? "/" : "");
    TableWidgetItem * filePathItem = new TableWidgetItem( QDir::toNativeSeparators( relPath));
    filePathItem->setFlags( filePathItem->flags() ^ Qt::ItemIsEditable);
    filePathItem->setData( Qt::UserRole, QDir::toNativeSeparators( filePath));
    auto fpTooltip = "{SF} = " + dirComboBox->currentText();
    if (!fpTooltip.endsWith(QDir::separator()) && finfo.isDir())
         fpTooltip += QDir::separator();
    filePathItem->setData(Qt::ToolTipRole, QVariant(fpTooltip));

    auto fileExt = !finfo.suffix().isEmpty() ? "." + finfo.suffix() : "";
    if (fileExt == fileName || finfo.isDir())
        fileExt = "";
    auto fileExtItem = new TableWidgetItem(fileExt);
    fileExtItem->setFlags( fileExtItem->flags() ^ Qt::ItemIsEditable);
    fileExtItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    //const QString dateModStr = finfo.lastModified().toString( Qt::SystemLocaleShortDate); //( Qt::ISODate); ( "yyyy.MM.dd HH:mm:ss");
    QTableWidgetItem * dateModItem = new QTableWidgetItem();
    dateModItem->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    dateModItem->setFlags( dateModItem->flags() ^ Qt::ItemIsEditable);
    dateModItem->setData( Qt::DisplayRole, finfo.lastModified());

    const double sizeKB = finfo.size() > 0 && finfo.size() < 104 ?
                            0.1 : double(finfo.size()) / double(1024);
    const QString sizeKBqs = QString::number(sizeKB, 'f', 1);
    const double sizeKBround = sizeKBqs.toDouble();
    QString sizeText;
    sizeToHumanReadable( static_cast<quint64>(finfo.size()), sizeText);
    QTableWidgetItem * sizeItem = new QTableWidgetItem();
    sizeItem->setFlags( sizeItem->flags() ^ Qt::ItemIsEditable);
    sizeItem->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    sizeItem->setData( Qt::DisplayRole, finfo.isFile() ? QVariant(sizeKBround) : QVariant(""));
    sizeItem->setData( Qt::ToolTipRole, finfo.isFile() ? QVariant(sizeText) : QVariant(""));

    auto fsTypeItem = new TableWidgetItem(FsItemType(finfo));
    fsTypeItem->setFlags(fsTypeItem->flags() ^ Qt::ItemIsEditable);
    fsTypeItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    auto ownerItem = new TableWidgetItem(finfo.owner());
    ownerItem->setFlags(ownerItem->flags() ^ Qt::ItemIsEditable);
    ownerItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ownerItem->setData(Qt::ToolTipRole, QVariant("Username of the item's owner."));

    if (!_stopped) {
        QVector<QTableWidgetItem*> rowItems;
        rowItems.reserve(N_COL);
        rowItems.append(filePathItem);
        rowItems.append(fileNameItem);
        rowItems.append(sizeItem);
        rowItems.append(dateModItem);
        rowItems.append(fileExtItem);
        rowItems.append(fsTypeItem);
        rowItems.append(ownerItem);

        itemBuffer.append(rowItems);
        if (itemBuffer.size() >= BATCH_SIZE) {
            flushItemBuffer();
            filesTable->scrollToBottom();
            filesFoundLabel->setText(tr("Found %1 items so far...").arg(filesTable->rowCount()));
            processEvents();
        }
    }
    processEvents();
  }
  catch(...) { Q_ASSERT(false); }
}

void MainWindow::flushItemBuffer() {
    if (itemBuffer.isEmpty())
        return;

    filesTable->setUpdatesEnabled(false);
    const int startRow = filesTable->rowCount();
    filesTable->setRowCount(startRow + int(itemBuffer.size()));

    for (int i = 0; i < itemBuffer.size(); ++i) {
        const auto& rowItems = itemBuffer[i];
        for (int col = 0; col < rowItems.size(); ++col) {
            filesTable->setItem(startRow + i, col, rowItems[col]);
            filesTable->setRowHeight(startRow + i, 45);
        }
    }
    filesTable->setUpdatesEnabled(true);
    itemBuffer.clear();
}

void MainWindow::createFilesTable()
{
  try
  {
    filesTable = new QTableWidget(0, N_COL, this);
    filesTable->setParent(this);

    filesTable->setWordWrap(true);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    filesTable->setAlternatingRowColors(true);
    filesTable->sortByColumn(-1, Qt::AscendingOrder);
    filesTable->setSortingEnabled(false);
    filesTable->setShowGrid(true);

    filesTable->verticalHeader()->hide();
    filesTable->verticalHeader()->setSectionResizeMode( QHeaderView::Interactive);

    static QStringList labels;
    labels.reserve(N_COL);
    labels.append(QString("Relative path"));
    labels.append(QString("Name"));
    labels.append(QString("Size [KB]"));
    labels.append(QString("Date modified"));
    labels.append(QString("Extension"));
    labels.append(QString("Item kind"));
    labels.append(QString("Owner"));
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive);

    int col = 0;
#if defined(Q_OS_MAC)
    filesTable->setColumnWidth(col++, 400);
    filesTable->setColumnWidth(col++, 180);
    filesTable->setColumnWidth(col++,  80);
    filesTable->setColumnWidth(col++, 150);
    filesTable->setColumnWidth(col++,  80);
    filesTable->setColumnWidth(col++, 130);
    filesTable->setColumnWidth(col,    60);
    // filesTable->horizontalHeader()->setSectionResizeMode(N_COL-1, QHeaderView::Stretch);
#elif defined (Q_OS_WIN)
    filesTable->setColumnWidth(col++, 320);
    filesTable->setColumnWidth(col++, 140);
    filesTable->setColumnWidth(col++,  70);
    filesTable->setColumnWidth(col++, 140);
    filesTable->setColumnWidth(col++,  80);
    filesTable->setColumnWidth(col++, 130);
    filesTable->setColumnWidth(col,    60);
    // filesTable->horizontalHeader()->setSectionResizeMode(N_COL-1, QHeaderView::Stretch);
#else
    filesTable->setColumnWidth(col++, 320);
    filesTable->setColumnWidth(col++, 140);
    filesTable->setColumnWidth(col++,  70);
    filesTable->setColumnWidth(col++, 150);
    filesTable->setColumnWidth(col++,  80);
    filesTable->setColumnWidth(col++, 130);
    filesTable->setColumnWidth(col,    60);
    // filesTable->horizontalHeader()->setSectionResizeMode(N_COL-1, QHeaderView::Stretch);
#endif

    //modifyFont(filesTable, +1.0, true, false, false);

    bool
    c = connect( filesTable, SIGNAL(cellActivated(int,int)),
                 this, SLOT(openFileOfItem(int,int))); Q_ASSERT(c); (void)c;
    c = connect( filesTable, SIGNAL(itemSelectionChanged()),
                 this, SLOT(itemSelectionChanged())); Q_ASSERT(c); (void)c;

    filesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    const auto conn = connect(filesTable, &QTableWidget::customContextMenuRequested,
                              this, &MainWindow::showContextMenu);
    //c = connect(filesTable, SIGNAL(customContextMenuRequested(const QPoint &)),
    //            this,         SLOT(showContextMenu(const QPoint &)));  Q_ASSERT(c);
  }
  catch (...) { /*Q_ASSERT(false)*/; }
}

void MainWindow::openFileOfItem( int row, int /* column */)
{
    QTableWidgetItem * item = filesTable->item(row, RELPATH_COL_IDX);

    QDesktopServices::openUrl( QUrl::fromLocalFile( item->data(Qt::UserRole).toString() )); //( item->text()));
}

void MainWindow::itemSelectionChanged()
{
    deleteButton->setEnabled(_stopped && filesTable->selectedItems().count() > 0);
    shredButton->setEnabled( _stopped && filesTable->selectedItems().count() > 0);
}

void MainWindow::createContextMenu()
{
    try
    {
        contextMenu = new QMenu(this);  // Set parent to ensure proper cleanup

        openRunAct = contextMenu->addAction(OvSk_FsOp_OPENRUN_ACT_TXT);
        openContaingFolderAct = contextMenu->addAction(eCod_OPEN_CONT_FOLDER_ACT_TXT);
        copyPathAct = contextMenu->addAction(eCod_COPY_PATH_ACT_TXT);
        contextMenu->addSeparator();
        getSizeAct = contextMenu->addAction(eCod_GET_SIZE_ACT_TXT);
        propertiesAct = contextMenu->addAction(eCod_PROPERTIES_ACT_TXT);

        // Connect using new syntax
        connect(openRunAct, &QAction::triggered, this, &MainWindow::openRunSlot);
        connect(openContaingFolderAct, &QAction::triggered, this, &MainWindow::openContainingFolderSlot);
        connect(copyPathAct, &QAction::triggered, this, &MainWindow::copyPathSlot);
        connect(getSizeAct, &QAction::triggered, this, &MainWindow::getSizeSlot);
        connect(propertiesAct, &QAction::triggered, this, &MainWindow::propertiesSlot);
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::showContextMenu(const QPoint& point)
{
    try
    {
        if (!contextMenu) {
            qDebug() << "Context menu not created, nullptr.";
            return;
        }
        // Get the item at the click position
        QTableWidgetItem* item = filesTable->itemAt(point);
        if (!item) {
            qDebug() << "No item at click position.";
            return;
        }

        // Enable/disable actions based on selection
        bool hasSelection = !filesTable->selectedItems().empty();
        openRunAct->setEnabled(hasSelection);
        openContaingFolderAct->setEnabled(hasSelection);
        copyPathAct->setEnabled(hasSelection);
        getSizeAct->setEnabled(hasSelection);
        propertiesAct->setEnabled(hasSelection);

        // Show the menu at the correct global position
        contextMenu->popup(filesTable->viewport()->mapToGlobal(point));
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::openRunSlot() {
    try {
        const auto selectedItems = filesTable->selectedItems();
        if (selectedItems.empty()) {
            return;
        }
        const auto item = selectedItems.first();
        const auto finfo = QFileInfo(item->data(Qt::UserRole).toString());
        // absoluteFilePath() is good for both files and folders
        const auto url = QUrl::fromLocalFile(finfo.absoluteFilePath());
        QDesktopServices::openUrl(url);
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::openContainingFolderSlot() {
    try {
        const auto selectedItems = filesTable->selectedItems();
        if (selectedItems.empty()) {
            return;
        }
        const auto item = selectedItems.first();
        const auto finfo = QFileInfo(item->data(Qt::UserRole).toString());
        // absolutePath() is the containing folder (i.e. absolute path
        // without the file/folder name)
        const auto url = QUrl::fromLocalFile(finfo.absolutePath());
        QDesktopServices::openUrl(url);
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::copyPathSlot() {
    try {
        const auto selectedItems = filesTable->selectedItems();
        if (selectedItems.empty()) {
            return;
        }
        auto clipboard = QApplication::clipboard();
        if (!clipboard) {
            qDebug() << "copyPathSlot: Clipboard not available.";
            return;
        }
        const auto item = selectedItems.first();
        const auto finfo = QFileInfo(item->data(Qt::UserRole).toString());
        clipboard->setText(QDir::toNativeSeparators(finfo.absoluteFilePath()));
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::getSizeSlot() {
    try {
        const auto selectedItems = filesTable->selectedItems();
        if (selectedItems.empty()) {
            return;
        }
        const auto item = selectedItems.first();
        const auto finfo = QFileInfo(item->data(Qt::UserRole).toString());
        const auto nativePath = QDir::toNativeSeparators(finfo.absoluteFilePath());
        const auto [count, size] = scanner->deepCountSize(nativePath);
        QString sizeStr;
        const auto res = sizeToHumanReadable(size, sizeStr); (void)res;
        QMessageBox::information(this, OvSk_FsOp_APP_NAME_TXT,
                                 tr("%1\n\nItem count: %2\nTotal size: %3")
                                    .arg(finfo.absoluteFilePath())
                                    .arg(count)
                                    .arg(sizeStr));
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::propertiesSlot() {
    try
    {
        auto selectedItems = filesTable->selectedItems();
        if (selectedItems.empty()) {
            return;
        }
        const auto item = selectedItems.first();
        const auto finfo = QFileInfo(item->data(Qt::UserRole).toString());
        const auto filePath = QDir::toNativeSeparators(finfo.absoluteFilePath());
#if defined(Q_OS_LINUX)
        QProcess::startDetached("xdg-open", QStringList() << filePath);
#elif defined(Q_OS_MACOS)
        showFileProperties(filePath.toUtf8().constData());
#elif defined(Q_OS_WIN)
        LPCWSTR file = (const wchar_t*)filePath.utf16();
        SHELLEXECUTEINFO sei = { sizeof(sei) };
        sei.lpVerb = L"properties";
        sei.lpFile = file;
        sei.nShow = SW_SHOW;
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        if (!ShellExecuteEx(&sei)) {
            qDebug() << "propertiesSlot: Failed to open properties dialog";
        }
#endif
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::keyReleaseEvent( QKeyEvent* ev) {
    try
    {
        if (ev->key() == Qt::Key_Escape) {
            _stopped = true;
            ev->accept();
        }
        else if (ev->key() == Qt::Key_Delete) {
            deleteBtnClicked();
            ev->accept();
        }
        else
            QMainWindow::keyReleaseEvent(ev);
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::unlimSubDirDepthToggled(bool /*checked*/)
{
    try {
        _unlimSubDirDepth = unlimSubDirDepthBtn->isChecked();
        if (_unlimSubDirDepth) {
            bool maxValid = false;
            _maxSubDirDepth =  maxSubDirDepthEdt->text().toInt(&maxValid);
            if (!maxValid) {
                _maxSubDirDepth = 0;
                maxSubDirDepthEdt->setText("0");
            }
            maxSubDirDepthEdt->setEnabled(false);
            maxSubDirDepthEdt->setText("");
        }
        else {
            maxSubDirDepthEdt->setEnabled(true);
            maxSubDirDepthEdt->setText(QString("%1").arg(_maxSubDirDepth));
        }
    }
    catch (...) { Q_ASSERT(false); }
}

void MainWindow::showAboutDialog() {
    AboutDialog dialog(this);
    dialog.exec();
}
void MainWindow::showHelpDialog() {
    HelpDialog dialog(this);
    dialog.exec();
}

void MainWindow::deepScanFolderOnThread(const QString& startPath, const int maxDepth)
{
    // Create scanner and move it to the thread
    auto scanThread = new QThread(this);
    scanner = new FolderScanner;

    // Will only start the thread if initialization succeeds
    if (!findFilesPrep(scanner)) {
        delete scanner;
        delete scanThread;
        return;
    }

    // Moving worker object pointer to thread (scanner below)
    // only sets which thread (scanThread) will execute worker's slots.
    scanner->moveToThread(scanThread);

    // First: Connect your function to handle thread completion
    connect(scanThread, &QThread::finished, this, &MainWindow::scanThreadFinished);

    // Then: Connect the cleanup connections after your handler
    connect(scanThread, &QThread::finished, scanner, &QObject::deleteLater);
    connect(scanThread, &QThread::finished, scanThread, &QObject::deleteLater);

    // Connect signals/slots
    connect(scanThread, &QThread::started,
        [this, startPath, maxDepth]() { this->scanner->deepScan(startPath, maxDepth); });

    connect(scanner, &FolderScanner::itemFound, this, &MainWindow::itemFound);
    connect(scanner, &FolderScanner::itemRemoved, this, &MainWindow::itemRemoved);
    connect(scanner, &FolderScanner::progressUpdate, this, &MainWindow::progressUpdate);

    connect(scanner, &FolderScanner::scanComplete, scanThread, &QThread::quit);
    connect(scanner, &FolderScanner::scanCancelled, scanThread, &QThread::quit);

    // Connect cancel button
    //connect(cancelButton, &QPushButton::clicked, scanner, &FolderScanner::stop);

    // DO IT
    scanThread->start();
}

void MainWindow::scanThreadFinished()
{
    flushItemBuffer();
    setFilesFoundLabel(_stopped ? "INTERRUPTED. " : "COMPLETED. ");
    setStopped(true);
    filesTable->sortByColumn(-1, Qt::AscendingOrder);
    filesTable->setSortingEnabled(true);
}

void MainWindow::itemFound(const QString& path, const QFileInfo& info) {
    if (!_stopped)
        appendItemToTable(path, info);
}

void MainWindow::itemRemoved(int row, quint64 count, quint64 size, int nbrDeleted) {
    if (!_stopped) {
        filesTable->removeRow(row);
        _foundCount -= count;
        _foundSize -= size;
        _totCount -= count;
        _totSize -= size;
        (void)nbrDeleted; //if ((nbrDeleted % 5) == 0)
        emit filesTable->itemSelectionChanged();
    }
}

void MainWindow::progressUpdate(quint64 foundCount, quint64 foundSize, quint64 totCount, quint64 totSize) {
    _foundCount = foundCount;
    _foundSize = foundSize;
    _totCount = totCount;
    _totSize = totSize;
}

void MainWindow::deepRemoveFilesOnThread_AmzQ(const QStringList& pathsToRemove) {
    removerAmzQ_ = std::make_unique<AmzQ::FileRemover>(this);

    removerAmzQ_->removeFilesAndFolders02(
        pathsToRemove,
        // Progress callback
        [this](const QString& currentFile) {
            filesFoundLabel->setText("Removing: " + currentFile);
        },
        // Completion callback
        [this](bool success) {
            if (success) {
                filesFoundLabel->setText("Removal completed successfully");
            }
            else {
                filesFoundLabel->setText("Some files could not be removed");
            }
        }
    );
}

void MainWindow::deepRemoveFilesOnThread_Claude(const QStringList & pathsToRemove) {
    removerClaude_ = std::make_unique<Claude::FileRemover>();

    // Set up progress callback (can update UI)
    removerClaude_->setProgressCallback([this](const std::filesystem::path& path, bool success) {
        // Since this callback runs in a different thread, use Qt::QueuedConnection
        QMetaObject::invokeMethod(this, [this, pathStr = path.string(), success]() {
            if (success) {
                filesFoundLabel->setText(QString("Successfully removed: %1")
                    .arg(QString::fromStdString(pathStr)));
            }
            else {
                filesFoundLabel->setText(QString("Failed to remove: %1")
                    .arg(QString::fromStdString(pathStr)));
            }
            }, Qt::QueuedConnection);
        });

    for (const auto& path : pathsToRemove) {
        removerClaude_->queueForRemoval(path.toStdString());
    }
}
} // namespace Devonline
