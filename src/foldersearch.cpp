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

#if defined(_WIN32)|| defined(_WIN64)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#undef ERROR
#undef min
#undef max
#endif

#include "fileremover-v2.hpp"
#include "fileremover-v3.hpp"
#include "foldersearch.hpp"
#include "scanparams.hpp"
#include "aboutdialog.h"
#include "helpdialog.h"
#include "config.h"
#include "util.h"
#include "version.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <thread>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QThread>
#include <QProcess>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QtCore/QRegularExpression>
#include <QDebug>

#if defined(Q_OS_MAC)
extern "C" void showFileProperties(const char* filePath);
#endif

namespace Devonline
{
const int N_COL = 7;
const int RELPATH_COL_IDX = 0;
const int NAME_COL_IDX = 1;

static int BATCH_SIZE = 1'000;
QVector<QVector<QTableWidgetItem*>> itemBuffer;

using namespace std::chrono;
using namespace std::chrono_literals;

void MainWindow::stopAllThreads() {
    static constexpr auto sleepLen = 300ms;
    if (scanner) {
        scanner->stop();
        std::this_thread::sleep_for(sleepLen);
    }
    if (scanThread) {
        std::this_thread::sleep_for(sleepLen);
        scanThread->exit(0);
        for (int i = 0; !scanThread->isFinished() && i < 10 ; ++i) {
            std::this_thread::sleep_for(sleepLen);
        }
        if (!scanThread->isFinished()) {
            qDebug() << "MainWindow::stopAllThreads: calling thread TERMINATE";
            scanThread->terminate();
        }
        scanThread->wait();
    }
    if (removerFrv2) {
        removerFrv2->stop();
        std::this_thread::sleep_for(sleepLen);
    }
    if (removerFrv3) {
        removerFrv3->stop();
        std::this_thread::sleep_for(sleepLen);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    const auto shouldAllowClose = true; // TODO: review
    if (shouldAllowClose) {
        stopAllThreads();
        event->accept();
    }
    else {
        event->ignore();  // Prevents closing
    }
}

MainWindow::MainWindow( const QString& /*dirPath*/, QWidget* parent)
    : QMainWindow(parent)
    , _ignoreDirPathChange(false)
    , _origDirPath( QDir::toNativeSeparators( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory)))
    , _maxSubDirDepth(0)
    , _unlimSubDirDepth(true)
    , _stopped(true)
    , _removal(false)
    , _gettingSize(false)
{
    qRegisterMetaType<Devonline::MainWindow>("Devonline::MainWindow");
    prevEvents = 0;
    eventsTimer.start();
    prevProgress = 0;
    progressTimer.start();

    setWindowTitle(QString(OvSk_FsOp_APP_NAME_TXT) + " " + OvSk_FsOp_APP_VERSION_STR + " " + OvSk_FsOp_APP_BUILD_NBR_STR);
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
}

MainWindow::~MainWindow()
{
}

void MainWindow::createSubDirLayout()
{
    maxSubDirDepthLbl = new QLabel(tr("Max subfolder depth: "), this);
    setAllTips(maxSubDirDepthLbl, tr("Specify unlimited or maximum sub-folder depth."));
    unlimSubDirDepthBtn = new QRadioButton(tr("Unlimited "), this);
    unlimSubDirDepthBtn->setChecked(true);
    limSubDirDepthBtn = new QRadioButton(tr("Limited to: "), this);
    maxSubDirDepthEdt = new QLineEdit(this);
    maxSubDirDepthEdt->setEnabled(false);
    maxSubDirDepthEdt->setFixedWidth(60);
    QIntValidator* intValidator = new QIntValidator(this);
    intValidator->setBottom(0);
    maxSubDirDepthEdt->setValidator(intValidator);
    QButtonGroup* subDirDepthGrp = new QButtonGroup(this);
    subDirDepthGrp->setExclusive(true);
    subDirDepthGrp->addButton(unlimSubDirDepthBtn, 1);
    subDirDepthGrp->addButton(limSubDirDepthBtn,   1);

    subDirDepthLout = new QHBoxLayout(this);
    subDirDepthLout->addWidget( maxSubDirDepthLbl);
    subDirDepthLout->addWidget( unlimSubDirDepthBtn);
    subDirDepthLout->addWidget( limSubDirDepthBtn);
    subDirDepthLout->addWidget( maxSubDirDepthEdt);
    subDirDepthLout->addStretch();
    bool c = connect(unlimSubDirDepthBtn, &QRadioButton::toggled, this, &MainWindow::unlimSubDirDepthToggled); Q_ASSERT(c); (void)c;
}

void MainWindow::createItemTypeCheckLayout()
{
    // itmTypeLbl = new QLabel(tr("Types:"), this);
    // setAllTips(itmTypeLbl, eCod_SEARCH_BY_TYPE_TIP);
    filesCheck    = new QCheckBox(tr("&Files"), this);
    foldersCheck  = new QCheckBox(tr("Folders"), this);
    symlinksCheck = new QCheckBox(OvSk_FsOp_SYMLINKS_TXT, this);
    filesCheck->setChecked( true);
    foldersCheck->setChecked( true);
    symlinksCheck->setChecked( true);

    itmTypeCheckLout = new QHBoxLayout(this);
    itmTypeCheckLout->addWidget( filesCheck);
    itmTypeCheckLout->addWidget( foldersCheck);
    itmTypeCheckLout->addWidget( symlinksCheck);
    itmTypeCheckLout->addSpacing(40);
    itmTypeCheckLout->addLayout( subDirDepthLout);
    itmTypeCheckLout->addStretch();
}

void MainWindow::createNavigLayout()
{
    browseButton = new QToolButton(this);
    browseButton->setText(tr("Browse..."));
    setAllTips(browseButton, eCod_BROWSE_FOLDERS_TIP);
    bool c = connect(browseButton, SIGNAL(clicked()), this, SLOT(browseBtnClicked())); Q_ASSERT(c);

    goUpButton = new QToolButton(this);
    goUpButton->setText(tr("^"));
    setAllTips(goUpButton, eCod_BROWSE_GO_UP_TIP);
    c = connect(goUpButton, SIGNAL(clicked()), this, SLOT(goUpBtnClicked())); Q_ASSERT(c);

    navigLout = new QHBoxLayout(this);
    navigLout->addWidget( browseButton);
    navigLout->addWidget( goUpButton);
    navigLout->addStretch();

    findButton   = createButton(tr("&Search"),    SLOT(findBtnClicked()), this);
    deleteButton = createButton(tr("&Delete"),    SLOT(deleteBtnClicked()), this);
    // shredButton  = createButton(tr("Shred"),     SLOT(shredBtnClicked()), this);
    cancelButton = createButton(tr("S&top"),      SLOT(cancelBtnClicked()), this);
    modifyFont(findButton, +1.0, true, false, false);

    dirComboBox = createComboBoxFSys( _origDirPath, true, this);
    setAllTips(dirComboBox, OvSk_FsOp_TOP_DIR_TIP);
    modifyFont(dirComboBox, +0.0, true, false, false);
    c = connect( dirComboBox,    SIGNAL(editTextChanged(const QString &)),
                 this,           SLOT(dirPathEditTextChanged(const QString &))); Q_ASSERT(c);

    wordsLineEdit = new QLineEdit(this); //createComboBoxText();
    wordsLineEdit->setPlaceholderText("Search words");
    setAllTips(wordsLineEdit, OvSk_FsOp_CONTAINING_TEXT_TIP);
    modifyFont(wordsLineEdit, +0.0, true, false, false);
    matchCaseCheck = new QCheckBox(tr("&Match case"), this);
    setAllTips(matchCaseCheck, "Match or ignore the case of letters in search words. Does not affect file/folder names. ");

    wordsLout = new QHBoxLayout(this);
    wordsLout->addWidget(wordsLineEdit);
    wordsLout->addWidget(matchCaseCheck);

    namesLineEdit = new QLineEdit(this); //createComboBoxText();
    namesLineEdit->setPlaceholderText("File/Folder names");
    setAllTips(namesLineEdit, OvSk_FsOp_NAME_FILTERS_TIP);
    modifyFont(namesLineEdit, +0.0, true, false, false);
}

void MainWindow::createExclLayout()
{
    toggleExclBtn = new QToolButton(this);
    toggleExclBtn->setText(tr("-"));
    setAllTips(toggleExclBtn, eCod_SHOW_EXCL_OPTS_TIP);
    bool c = connect(toggleExclBtn, SIGNAL(clicked()), this, SLOT(toggleExclClicked())); Q_ASSERT(c); (void)c;

    exclFilesByTextCombo = new QLineEdit(this); //createComboBoxText();
    exclFilesByTextCombo->setPlaceholderText("Exclude files with words");
    setAllTips(exclFilesByTextCombo, eCod_EXCL_FILES_BY_CONTENT_TIP);
    modifyFont(exclFilesByTextCombo, +0.0, false, false, false);

    exclByFileNameCombo = new QLineEdit(this); //createComboBoxText();
    exclByFileNameCombo->setPlaceholderText("Exclude by partial file names");
    setAllTips(exclByFileNameCombo, eCod_EXCL_FILES_BY_NAME_TIP);
    modifyFont(exclByFileNameCombo, +0.0, false, false, false);

    exclByFolderNameCombo = new QLineEdit(this); //createComboBoxText();
    exclByFolderNameCombo->setPlaceholderText("Exclude by partial folder names");
    setAllTips(exclByFolderNameCombo, eCod_EXCL_FOLDERS_BY_NAME_TIP);
    modifyFont(exclByFolderNameCombo, +0.0, false, false, false);

    exclHiddenCheck = new QCheckBox(this);
    exclHiddenCheck->setChecked(false);
    exclHiddenCheck->setText("Exclude hidden");
    setAllTips(exclHiddenCheck, eCod_EXCL_HIDDEN_ITEMS);
    modifyFont(exclHiddenCheck, +0.0, false, false, false);
}

void MainWindow::createMainLayout()
{
    filesFoundLabel = new QLabel(this);
    modifyFont(filesFoundLabel, +0.0, true, false, false);

    QScrollArea* filesFoundScroll = new QScrollArea(this);
    filesFoundScroll->setWidget(filesFoundLabel);
    filesFoundScroll->setWidgetResizable(true);
    filesFoundScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    filesFoundScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    filesFoundScroll->setMaximumHeight(40);

    searchFolderLbl = new QLabel("Search folder {SF}:", this);
    searchFolderLbl->setToolTip("Folder to deeply search, a.k.a. {SF}");
    modifyFont(searchFolderLbl, +0.0, true, false, false);

    auto dirComboLayout = new QHBoxLayout(this);
    dirComboLayout->addWidget(searchFolderLbl);
    dirComboLayout->addWidget(dirComboBox);
    dirComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *buttonsLayout = new QHBoxLayout(this);
    buttonsLayout->addStretch();
    //buttonsLayout->addWidget(shredButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(findButton);

    int gridRowIdx = 0;
    QGridLayout *mainLayout = new QGridLayout(this);
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
    mainLayout->addWidget(filesFoundScroll,     gridRowIdx, 0, 1, 4);
    ++gridRowIdx;
    mainLayout->addLayout(buttonsLayout,        gridRowIdx, 0, 1, 4);
    ++gridRowIdx;

    centralWgt = new QWidget(this);
    setCentralWidget(centralWgt);
    centralWgt->setLayout(mainLayout);
    resize(1200, 700);
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
    filesFoundLabel->setText("");
    _opType = Devonline::Op::deletePerm;
    if (filesTable->selectedItems().empty()) {
        setFilesFoundLabel(OvSk_FsOp_SELECT_FOUNDFILES_TXT);
        #if !defined(Q_OS_MAC)
            QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
        #endif
        return;
    }
    if (!_stopped) {
        stopAllThreads();
    }
    setStopped(false);
    _removal = true;
    processEvents();
    IntQStringMap itemList;
    getSelectedItems(itemList);
    bool maxValid = false;
    _maxSubDirDepth = maxSubDirDepthEdt->text().toInt(&maxValid);
    if (!maxValid)
        _maxSubDirDepth = 0;
    const auto maxDepth = unlimSubDirDepthBtn->isChecked() ? -1 : _maxSubDirDepth;
    opStart = steady_clock::now();

    // REMOVE ITEMS
    if (maxDepth < 0) {
        // UNLIMITED
        // different impl: deepRemoveFilesOnThread_Frv3(itemList);
        // old impl: removeItems(itemList);
        deepRemoveFilesOnThread_Frv2(itemList);
    }
    else {
        // LIMITED
        deepRemoveLimitedOnThread(itemList, maxDepth);
    }
}

void MainWindow::getSelectedItems(IntQStringMap& itemList)
{
    const QList<QTableWidgetItem*> selectedItems = filesTable->selectedItems();
    for (const auto item : selectedItems)
    {
        if (filesTable->column( item) == RELPATH_COL_IDX)
        {
            if (_stopped)
                return;
            const auto info = QFileInfo(item->data(Qt::UserRole).toString());
            const auto path = QDir::toNativeSeparators(info.absoluteFilePath());
            const auto row = filesTable->row(item);
            itemList.insert(std::make_pair(row, path));
            processEvents();
        }
    }
    qDebug() << "NBR SELECTED" << itemList.size();
}

void MainWindow::shredBtnClicked()
{
    filesFoundLabel->setText("");
    _opType = Devonline::Op::shredPerm;
    if (filesTable->selectedItems().empty()) {
        setFilesFoundLabel(OvSk_FsOp_SELECT_FOUNDFILES_TXT);
        #if !defined(Q_OS_MAC)
            QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
        #endif
        // return;
    }
    setStopped(false);
    _removal = true;
    opStart = steady_clock::now();

    // TODO: IMPLEMENT Shredding

    emit filesTable->itemSelectionChanged();
}

void MainWindow::cancelBtnClicked()
{
    // scanThreadFinished() does almost everything necessary, so we don't need to do much here
    stopAllThreads();
    if (_removal) {
        removalComplete(false);
    }
    _stopped = true;
}

void MainWindow::SetDirPath( const QString& dirPath)
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

void MainWindow::Clear()
{
    filesTable->setRowCount(0);
    filesTable->hide();
    filesTable->show();
    itemBuffer.clear();
    rowsToRemove_.clear();
    filesFoundLabel->setText("");
    _dirCount = 0;
    _foundCount = 0;
    _foundSize = 0;
    _symlinkCount = 0;
    _totCount = 0;
    _totSize = 0;
    _nbrDeleted = 0;
    processEvents();
}

void MainWindow::setStopped(bool stopped)
{
    _stopped = stopped;
    if (_stopped) {
        //filesTable->update();
        filesTable->setSortingEnabled(true);
        filesTable->sortByColumn(-1, Qt::AscendingOrder);
    }
    std::this_thread::sleep_for(200ms);

    findButton->setEnabled(     _stopped);
    deleteButton->setEnabled(   _stopped && filesTable->selectedItems().count() > 0);
    // shredButton->setEnabled(    _stopped && filesTable->selectedItems().count() > 0);
    cancelButton->setEnabled(  !_stopped);
    searchFolderLbl->setEnabled(_stopped);
    namesLineEdit->setEnabled(  _stopped);
    dirComboBox->setEnabled(    _stopped);
    // itmTypeLbl->setEnabled(     _stopped);
    filesCheck->setEnabled(     _stopped);
    foldersCheck->setEnabled(   _stopped);
    symlinksCheck->setEnabled(  _stopped);
    unlimSubDirDepthBtn->setEnabled(_stopped);
    limSubDirDepthBtn->setEnabled(  _stopped);
    maxSubDirDepthEdt->setEnabled(  _stopped && limSubDirDepthBtn->isChecked());
    maxSubDirDepthLbl->setEnabled(  _stopped);
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

QString MainWindow::getElapsedTimeStr() const
{
    const auto diff = opEnd - opStart;
    const auto totSecs = duration_cast<seconds>(diff).count();
    const auto hr  = totSecs / 3600;
    const auto min = (totSecs % 3600) / 60;
    const auto sec = totSecs % 60;
    const auto secStr = QString::number(sec);
    const auto minStr = QString::number(min);
    const auto hrStr  = QString::number(hr);
    QString timeStr;
    if (hr > 0)
        timeStr = " hr " + minStr + " min " + secStr + " sec";
    else if (min > 0)
        timeStr = minStr + " min " + secStr + " sec";
    else {
        const auto ms = duration_cast<milliseconds>(diff).count();
        const auto secs = (double)ms / 1000.0;
        timeStr = QString::number(secs, 'f', 2) + " sec";
    }
    return timeStr;
}

void MainWindow::setFilesFoundLabel(const QString& prefix, bool appendCounts /*= true*/)
{
    if (_gettingSize) {
        return;
    }
    if (_foundCount == 0) {
        _foundSize = 0;
    }
    opEnd = steady_clock::now();
    const auto elapsedStr = getElapsedTimeStr();
    const auto totItemsSizeStr = sizeToHumanReadable(_totSize);
    const auto foundItemsSizeStr = sizeToHumanReadable(_foundSize);
    auto foundLabelText = prefix;
    if (appendCounts) {
        foundLabelText += " | ";
        foundLabelText =
            foundLabelText + tr("%1 matching files (%2), %3 folders, %4 %5, total %6; took %7")
                                .arg(_foundCount)
                                .arg(foundItemsSizeStr)
                                .arg(_dirCount)
                                .arg(_symlinkCount)
                                .arg(OvSk_FsOp_SYMLINKS_LOW)
                                .arg(filesTable->rowCount())
                                .arg(elapsedStr);
                                //.arg(_totCount)
                                //.arg(totItemsSizeStr);
    }
    filesFoundLabel->setText(foundLabelText);
    qDebug() << foundLabelText;
    if ((_foundCount + _dirCount + _symlinkCount) != quint64(filesTable->rowCount())) {
        qDebug() << "WARNING: COUNTS" << (_foundCount + _dirCount + _symlinkCount)
                 << "!= TABLE ROW COUNT" << filesTable->rowCount();
    }
}

bool MainWindow::findFilesPrep()
{
    if (!_stopped) {
        stopAllThreads();
    }
    Clear();
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

    scanner = std::make_shared<FolderScanner>();
    scanner->params.itemTypeFilter = _itemTypeFilter;
    scanner->params.inclFiles = filesCheck->isChecked();
    scanner->params.inclFolders = foldersCheck->isChecked();
    scanner->params.inclSymlinks = symlinksCheck->isChecked();
    scanner->params.exclHidden = exclHiddenCheck->isChecked();

    _matchCase = matchCaseCheck->isChecked();
    scanner->params.matchCase = _matchCase;

    const auto dirComboCurrent = dirComboBox->currentText().trimmed();
    if (dirComboCurrent.length() == 0) {
        const auto msg = QString("Select a folder to search please.");
        qDebug() << msg;
        setFilesFoundLabel(msg);
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, msg);
        #endif
        setStopped(true);
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
        const auto msg = OvSk_FsOp_DIR_NOT_EXISTS_TXT + _origDirPath;
        qDebug() << msg;
        setFilesFoundLabel(msg);
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, msg);
        #endif
        setFilesFoundLabel("SEARCH FOLDER NOT FOUND: " + _origDirPath, false);
        setStopped(true);
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

    _fileNameFilter = namesLineEdit->text().trimmed();
    scanner->params.nameFilters = _fileNameFilter.split(" ", Qt::SkipEmptyParts);

    BATCH_SIZE = (_searchWords.isEmpty() &&
                  _exclusionWords.isEmpty() &&
                  scanner->params.nameFilters.isEmpty()) ? 1'000 : 1;

    bool maxValid = false;
    _maxSubDirDepth =  maxSubDirDepthEdt->text().toInt(&maxValid);
    if (!maxValid)
        _maxSubDirDepth = 0;
    _unlimSubDirDepth = unlimSubDirDepthBtn->isChecked();

    // Create scan thread (QThread) and FolderScanner
    scanThread = std::make_shared<QThread>(this);
    scanThread->setObjectName("FolderScannerThread");

    // Moving worker object pointer to thread (scanner pointer below)
    // only sets which thread (scanThread) will execute worker's slots.
    scanner->moveToThread(scanThread.get());
    return true;
}

void MainWindow::findBtnClicked()
{
    if (!_stopped) {
        return;
    }
    opStart = steady_clock::now();
    if (!filesCheck->isChecked() && !foldersCheck->isChecked() && !symlinksCheck->isChecked()) {
        setFilesFoundLabel(OvSk_FsOp_SELECT_ITEM_TYPE_TXT);
        #if !defined(Q_OS_MAC)
            QMessageBox::warning(this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_ITEM_TYPE_TXT);
        #endif
        return;
    }
    Cfg::St().setValue(Cfg::origDirPathKey, QDir::toNativeSeparators(dirComboBox->currentText()));

    //createFilesTable();

    // Will only start the thread if preparation succeeds
    if (!findFilesPrep()) {
        return;
    }

    setStopped(false);

    // Moving worker object pointer to thread (scanner pointer below)
    // only sets which thread (scanThread) will execute worker's slots.
    scanner->moveToThread(scanThread.get());

    deepScanFolderOnThread(_origDirPath, _unlimSubDirDepth ? -1 : _maxSubDirDepth);
}

inline void MainWindow::processEvents()
{
    const auto elapsed = eventsTimer.elapsed();
    const auto diff = elapsed - prevEvents;
    if (diff >= 1'000) {  // msec
        prevEvents = elapsed;
        qApp->processEvents(QEventLoop::AllEvents, 300);
    }
}

QPushButton* MainWindow::createButton(const QString& text, const char* member, QWidget* parent)
{
    QPushButton *button = new QPushButton(text, parent);
    const bool c = connect(button, SIGNAL(clicked()), this, member); Q_ASSERT(c); (void)c;
    return button;
}

#if defined(_WIN32) || defined(_WIN64)
    #define eCod_TOP_ROOT_PATH "C:\\"
#else
    #define eCod_TOP_ROOT_PATH "/"
#endif

void MainWindow::dirPathEditTextChanged(const QString& newText)
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
        #if defined(_WIN32) || defined(_WIN64)
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
    _editTextTimeDiff.restart();
}

void MainWindow::completerTimeout()
{
    if (dirComboBox->completer())
        dirComboBox->completer()->complete(); // shows the popup if the completion count > 0
}

QFileSystemModel* MainWindow::newFileSystemModel(QCompleter* completer, const QString& currentDir)
{
    fileSystemModel = new QFileSystemModel(completer);
    fileSystemModel->setReadOnly( true);
    fileSystemModel->setResolveSymlinks( false);
    fileSystemModel->setFilter( QDir::Dirs | QDir::Drives  | QDir::Hidden  | QDir::System | QDir::NoDotAndDotDot);
    fileSystemModel->setRootPath(currentDir);
    //fileSystemModel->sort(0, Qt::AscendingOrder);
    return fileSystemModel;
}

QComboBox * MainWindow::createComboBoxFSys(const QString& text, bool setCompleter, QWidget* parent)
{
    QCompleter * completer = NULL;
    if (setCompleter)
    {
        completer = new QCompleter(parent);
        #if defined(_WIN32) || defined(_WIN64)
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

    QComboBox *comboBox = new QComboBox(parent);
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

QComboBox* MainWindow::createComboBoxText(QWidget* parent)
{
    QCompleter* completer = new QCompleter(parent);
    completer->setCaseSensitivity( Qt::CaseSensitive);
    completer->setModelSorting( QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode( QCompleter::InlineCompletion);
    completer->setMaxVisibleItems( 16);
    completer->setWrapAround( false);

    QComboBox* comboBox = new QComboBox(parent);
    comboBox->setEditable( true);
    comboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred);

    completer->setParent( comboBox);
    //completer->setModel( ? );
    comboBox->setCompleter( completer);

    return comboBox;
}

QString MainWindow::FsItemType(bool isFile, bool isDir, bool isSymlink, bool isHidden) const
{
    QString fsType;
    //if (finfo.isAlias())
    //    fsType = "Alias";
    //else if (isAppExecutionAlias(finfo.absoluteFilePath()))
    //    fsType = "App execution alias";
    //else if (isWindowsSymlink(finfo.absoluteFilePath()))
    //    fsType = "Windows symlink";
    if (isSymlink) {
        if (isDir)
            fsType = OvSk_FsOp_SYMLINK_TXT " to folder";
        else if (isFile)
            fsType = OvSk_FsOp_SYMLINK_TXT " to file";
        else
            fsType = OvSk_FsOp_SYMLINK_TXT;
    }
    else if (isDir)
        fsType = "Folder";
    else if (isFile)
        fsType = "File";
    else
        fsType = "Unknown";
    if (isHidden)
        fsType += " - hidden";
    return fsType;
}

void MainWindow::appendItemToTable(const QString& filePath, const QFileInfo& finfo)
{
    if (_stopped)
        return;
    //processEvents();
    const auto isSymlink = isSymbolic(finfo);
    const auto isDir = finfo.isDir() && !isSymlink;
    const auto isFile = finfo.isFile() && !isSymlink;
    const auto isHidden = finfo.isHidden();
    const auto fsize = (quint64)finfo.size();
    if (isSymlink)
        _symlinkCount++;
    else if (isDir)
        _dirCount++;
    else if (isFile) {
        _foundCount++;
        _foundSize += fsize;
    }
    auto fileNameItem = new QTableWidgetItem();
    auto fileName = finfo.fileName();
    if (isSymlink && !finfo.symLinkTarget().isEmpty())
        fileName += " -> " + finfo.symLinkTarget();
    else if (isDir)
        fileName += QDir::separator();
    fileNameItem->setData(Qt::DisplayRole, QDir::toNativeSeparators(fileName));
    fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
    auto absFilePath = QDir::toNativeSeparators(finfo.absoluteFilePath());
    if (isDir)
        absFilePath += QDir::separator();
    fileNameItem->setData(Qt::ToolTipRole, QVariant(absFilePath));

    const QString fpath = QDir::toNativeSeparators( finfo.path());
    const auto dlen = QDir::toNativeSeparators( _origDirPath).length();
    QString relPath = (fpath.length() <= dlen) ? "" : fpath.right( fpath.length() - dlen);
    if (relPath.startsWith( QDir::separator()))
        relPath = relPath.right( relPath.length() - 1);
    relPath = "{SF}/" + relPath + (relPath.length() > 0 ? "/" : "");
    auto filePathItem = new QTableWidgetItem( QDir::toNativeSeparators( relPath));
    filePathItem->setFlags( filePathItem->flags() ^ Qt::ItemIsEditable);
    filePathItem->setData( Qt::UserRole, QDir::toNativeSeparators( filePath));
    auto fpTooltip = QDir::toNativeSeparators(finfo.absolutePath());
    if (!fpTooltip.endsWith(QDir::separator()))
            fpTooltip += QDir::separator();
    filePathItem->setData(Qt::ToolTipRole, QVariant(fpTooltip));

    auto fileExt = !finfo.suffix().isEmpty() ? "." + finfo.suffix() : "";
    if (fileExt == fileName || isDir)
        fileExt = "";
    auto fileExtItem = new QTableWidgetItem(fileExt);
    fileExtItem->setFlags( fileExtItem->flags() ^ Qt::ItemIsEditable);
    fileExtItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    //const QString dateModStr = finfo.lastModified().toString( Qt::SystemLocaleShortDate); //( Qt::ISODate); ( "yyyy.MM.dd HH:mm:ss");
    QTableWidgetItem * dateModItem = new QTableWidgetItem();
    dateModItem->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    dateModItem->setFlags( dateModItem->flags() ^ Qt::ItemIsEditable);
    dateModItem->setData( Qt::DisplayRole, finfo.lastModified());

    const double sizeKB = fsize > 0 && fsize < 104 ?
                            0.1 : double(fsize) / double(1024);
    const QString sizeKBqs = QString::number(sizeKB, 'f', 1);
    const double sizeKBround = sizeKBqs.toDouble();
    const auto sizeText = sizeToHumanReadable(fsize);
    QTableWidgetItem* sizeItem = new QTableWidgetItem();
    sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);
    sizeItem->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    sizeItem->setData(Qt::DisplayRole, (isDir || isSymlink) ? QVariant("") : QVariant(sizeKBround));
    sizeItem->setData(Qt::ToolTipRole, (isDir || isSymlink) ? QVariant("") : QVariant(sizeText));

    auto fsTypeItem = new QTableWidgetItem(FsItemType(isFile, isDir, isSymlink, isHidden));
    fsTypeItem->setFlags(fsTypeItem->flags() ^ Qt::ItemIsEditable);
    fsTypeItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    auto ownerItem = new QTableWidgetItem(finfo.owner());
    ownerItem->setFlags(ownerItem->flags() ^ Qt::ItemIsEditable);
    ownerItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ownerItem->setData(Qt::ToolTipRole, QVariant("Username of the item's owner."));

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
        //filesTable->scrollToBottom();
    }
}

void MainWindow::flushItemBuffer() {
    if (itemBuffer.isEmpty())
        return;
    QString lastPath;
    {
        UpdateBlocker ub{ filesTable };
        const auto startRow = filesTable->rowCount();
        filesTable->model()->insertRows(startRow, int(itemBuffer.size()));  // Qt row index is int
        const auto buffSize = itemBuffer.size();
        for (int i = 0; i < buffSize; ++i) {
            const auto& rowItems = itemBuffer[i];
            for (int col = 0; col < rowItems.size(); ++col) {
                filesTable->setItem(startRow + i, col, rowItems[col]);
                filesTable->setRowHeight(startRow + i, 50);
            }
            if (i == (buffSize - 1)) {
                lastPath = rowItems[RELPATH_COL_IDX]->data(Qt::UserRole).toString() +
                           QDir::separator() +
                           rowItems[NAME_COL_IDX]->data(Qt::DisplayRole).toString();
            }
        }
    } // ub goes OUT OF SCOPE here, table updates and signals are enabled
    itemBuffer.clear();
    const auto rowCount = (quint64)filesTable->rowCount();
    if ((_foundCount + _dirCount + _symlinkCount) < rowCount) {
        qDebug() << "ERROR: (_foundCount + _dirCount + _symlinkCount)" <<
                            (_foundCount + _dirCount + _symlinkCount) << 
                            "< rowCount" << rowCount;
    }
    if (!_gettingSize) {
        filesFoundLabel->setText(QString("%1 matching files, %2 folders, %3 %4...  Searching through %5")
            .arg(_foundCount)
            .arg(_dirCount)
            .arg(_symlinkCount)
            .arg(OvSk_FsOp_SYMLINKS_TXT)
            .arg(QDir::toNativeSeparators(lastPath)));
    }
    //qDebug() << "Item buffer flushed, current row count " << rowCount;
    //processEvents();
}

void MainWindow::createFilesTable()
{
    //if (filesTable)
    //    delete filesTable;
    filesTable = new QTableWidget(0, N_COL, this);
    filesTable->setParent(this);
    filesTable->setColumnCount(N_COL);

    filesTable->setWordWrap(true);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    filesTable->setAlternatingRowColors(true);
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
#elif defined(_WIN32) || defined(_WIN64)
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
}

void MainWindow::openFileOfItem( int row, int /* column */)
{
    QTableWidgetItem * item = filesTable->item(row, RELPATH_COL_IDX);

    QDesktopServices::openUrl( QUrl::fromLocalFile( item->data(Qt::UserRole).toString() )); //( item->text()));
}

void MainWindow::itemSelectionChanged()
{
    deleteButton->setEnabled(_stopped && filesTable->selectedItems().count() > 0);
    // shredButton->setEnabled( _stopped && filesTable->selectedItems().count() > 0);
}

void MainWindow::createContextMenu()
{
    contextMenu = new QMenu(this);  // Set parent to ensure proper cleanup

    openRunAct = contextMenu->addAction(OvSk_FsOp_OPENRUN_ACT_TXT);
    openContaingFolderAct = contextMenu->addAction(eCod_OPEN_CONT_FOLDER_ACT_TXT);
    copyPathAct = contextMenu->addAction(eCod_COPY_PATH_ACT_TXT);
    #if !defined(Q_OS_MAC)
        contextMenu->addSeparator();
        getSizeAct = contextMenu->addAction(eCod_GET_SIZE_ACT_TXT);      // Not available on Mac
        propertiesAct = contextMenu->addAction(eCod_PROPERTIES_ACT_TXT); // ditto
    #else
        getSizeAct = nullptr;
        propertiesAct = nullptr;
    #endif

    // Connect using new syntax
    connect(openRunAct, &QAction::triggered, this, &MainWindow::openRunSlot);
    connect(openContaingFolderAct, &QAction::triggered, this, &MainWindow::openContainingFolderSlot);
    connect(copyPathAct, &QAction::triggered, this, &MainWindow::copyPathSlot);
    #if !defined(Q_OS_MAC)
        connect(getSizeAct, &QAction::triggered, this, &MainWindow::getSizeSlot);
        connect(propertiesAct, &QAction::triggered, this, &MainWindow::propertiesSlot);
    #endif
}

void MainWindow::showContextMenu(const QPoint& point)
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
    #if !defined(Q_OS_MAC)
        getSizeAct->setEnabled(hasSelection && _stopped);
        propertiesAct->setEnabled(hasSelection);
    #endif

    // Show the menu at the correct global position
    contextMenu->popup(filesTable->viewport()->mapToGlobal(point));
}

void MainWindow::openRunSlot() {
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

void MainWindow::openContainingFolderSlot() {
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

void MainWindow::copyPathSlot() {
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

void MainWindow::getSizeSlot() {
    const auto selectedItems = filesTable->selectedItems();
    _gettingSize = true;
    filesFoundLabel->setText("");
    setStopped(false);
    IntQStringMap itemList;
    getSelectedItems(itemList);
    getSizeOnThread(itemList);
}

void MainWindow::getSizeImpl(const IntQStringMap& itemList)
{
    uint64pair countNsize;
    const auto nbrItems = itemList.size();
    QString filePath;
    for (const auto& item : itemList) {
        filePath = item.second;
        const auto info = QFileInfo(filePath);
        // Do not follow symlinks
        if (info.isDir() && !isSymbolic(info)) {
            const auto [count, dirSize] = scanner->deepCountSize(filePath);
            countNsize.first += count;
            countNsize.second += dirSize;
        }
        else {
            countNsize.first++;
            countNsize.second += (quint64)info.size();
        }
    }
    emit scanner->scanComplete();

    // Use QMetaObject::invokeMethod to safely update UI from background thread
    QMetaObject::invokeMethod(this, [this, nbrItems, filePath, countNsize]() {
        // Update UI here
        this->setFilesFoundLabel(" ");
        const QString text = (nbrItems > 1) ? "Multiple files/folders" : filePath;
        const auto sizeStr = sizeToHumanReadable(countNsize.second);
        QMessageBox::information(this, OvSk_FsOp_APP_NAME_TXT,
            tr("%1\n\nItem count: %2\nTotal size: %3")
            .arg(text)
            .arg(countNsize.first)
            .arg(sizeStr));
    }, Qt::QueuedConnection);
}

void MainWindow::propertiesSlot() {
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
#elif defined(_WIN32) || defined(_WIN64)
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

void MainWindow::keyReleaseEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
        findBtnClicked();
        ev->accept();
    }
    else if (ev->key() == Qt::Key_Escape) {
        cancelBtnClicked();
        ev->accept();
    }
    else
        QMainWindow::keyReleaseEvent(ev);
}

void MainWindow::unlimSubDirDepthToggled(bool /*checked*/)
{
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

void MainWindow::showAboutDialog() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::showHelpDialog() {
    HelpDialog dialog(this);
    dialog.exec();
}

bool MainWindow::isHidden(const QFileInfo& finfo) const {
    return  finfo.isHidden() || finfo.fileName().startsWith('.');
}

void MainWindow::deepScanFolderOnThread(const QString& startPath, const int maxDepth)
{
    connect(scanThread.get(), &QThread::started, [this, startPath, maxDepth]() {
        scanner->deepScan(startPath, maxDepth);
    });
    connect(scanThread.get(), &QThread::finished, this, &MainWindow::scanThreadFinished);

    connect(scanner.get(), &FolderScanner::itemFound, this, &MainWindow::itemFound);
    connect(scanner.get(), &FolderScanner::itemSized, this, &MainWindow::itemSized);
    connect(scanner.get(), &FolderScanner::itemRemoved, this, &MainWindow::itemRemoved);
    connect(scanner.get(), &FolderScanner::progressUpdate, this, &MainWindow::progressUpdate);

    connect(scanner.get(), &FolderScanner::scanComplete, scanThread.get(), &QThread::quit);
    connect(scanner.get(), &FolderScanner::scanCancelled, scanThread.get(), &QThread::quit);

    // DO IT NOW
    scanThread->start();
}

void MainWindow::getSizeOnThread(const IntQStringMap& itemList)
{
    if (!_stopped) {
        stopAllThreads();
    }
    scanThread = std::make_shared<QThread>(this);
    scanThread->setObjectName("GetFolderSizeThread");

    // Moving worker object pointer to thread (scanner pointer below)
    // only sets which thread (scanThread) will execute worker's slots.
    scanner = std::make_shared<FolderScanner>();
    scanner->moveToThread(scanThread.get());

    connect(scanThread.get(), &QThread::started, [this, itemList]() {
        getSizeImpl(itemList);
    });
    connect(scanThread.get(), &QThread::finished, this, &MainWindow::scanThreadFinished);

    connect(scanner.get(), &FolderScanner::itemFound, this, &MainWindow::itemFound);
    connect(scanner.get(), &FolderScanner::itemSized, this, &MainWindow::itemSized);
    connect(scanner.get(), &FolderScanner::itemRemoved, this, &MainWindow::itemRemoved);
    connect(scanner.get(), &FolderScanner::progressUpdate, this, &MainWindow::progressUpdate);

    connect(scanner.get(), &FolderScanner::scanComplete, scanThread.get(), &QThread::quit);
    connect(scanner.get(), &FolderScanner::scanCancelled, scanThread.get(), &QThread::quit);

    // DO IT NOW
    scanThread->start();
}

void MainWindow::deepRemoveLimitedOnThread(const IntQStringMap& itemList, const int maxDepth)
{
    scanThread = std::make_shared<QThread>(this);
    scanThread->setObjectName("RemoveLimitedThread");

    // Moving worker object pointer to thread (scanner pointer below)
    // only sets which thread (scanThread) will execute worker's slots.
    scanner = std::make_shared<FolderScanner>();
    scanner->moveToThread(scanThread.get());

    connect(scanThread.get(), &QThread::started, [this, itemList, maxDepth]() {
        scanner->deepRemoveLimited(itemList, maxDepth);
    });
    connect(scanThread.get(), &QThread::finished, this, &MainWindow::scanThreadFinished);

    connect(scanner.get(), &FolderScanner::itemRemoved, this, &MainWindow::itemRemoved);
    connect(scanner.get(), &FolderScanner::progressUpdate, this, &MainWindow::progressUpdate);
    connect(scanner.get(), &FolderScanner::removalComplete, this, &MainWindow::removalComplete);

    connect(scanner.get(), &FolderScanner::removalCancelled, scanThread.get(), &QThread::quit);

    // DO IT NOW
    scanThread->start();
}

void MainWindow::scanThreadFinished()
{
    flushItemBuffer();
    std::this_thread::sleep_for(100ms);
    filesTable->update();
    std::this_thread::sleep_for(100ms);
    filesTable->setSortingEnabled(true);
    filesTable->sortByColumn(-1, Qt::AscendingOrder);
    if (!_removal) { // when _removal is true then removalComplete() sets the FilesFoundLabel
        setFilesFoundLabel(_stopped ? "INTERRUPTED" : "COMPLETED");
    }
    _removal = false;
    _gettingSize = false;
    setStopped(true);
}

void MainWindow::itemFound(const QString& path, const QFileInfo& info) {
    if (!_stopped)
        appendItemToTable(path, info);
}

void MainWindow::itemSized(const QString& path, const QFileInfo& info) {
    if (!_stopped)
        filesFoundLabel->setText(path + " " + sizeToHumanReadable((quint64)info.size()));
}

void MainWindow::itemRemoved(int row, quint64 /*count*/, quint64 /*size*/, quint64 /*nbrDeleted*/) {
    if (!_stopped) {
        filesTable->removeRow(row);
    }
}

void MainWindow::progressUpdate(const QString& path, quint64 totCount, quint64 totSize)
{
    _totCount = totCount;
    _totSize = totSize;
    if (!_stopped && !_gettingSize) {
        filesFoundLabel->setText(QString("%1 matching files, %2 folders, %3 %4...  Searching through %5")
            .arg(_foundCount)
            .arg(_dirCount)
            .arg(_symlinkCount)
            .arg(OvSk_FsOp_SYMLINKS_TXT)
            .arg(QDir::toNativeSeparators(path)));
    }
}

void MainWindow::removeRows()
{
    {
        UpdateBlocker ub{ filesTable };
        if (rowsToRemove_.size() == size_t(filesTable->rowCount())) {  // Qt row count is int
             filesTable->clearContents();
            //createFilesTable();
        }
        else {
            for (const auto rowRes : rowsToRemove_) {
                if (rowRes.first >= 0 && rowRes.second) {
                    auto item = filesTable->item(rowRes.first, RELPATH_COL_IDX);
                    delete item;
                    filesTable->removeRow(rowRes.first);
                }
            }
        }
    } // ub goes OUT OF SCOPE here, table updates and signals are enabled
    rowsToRemove_.clear();
    filesTable->sortByColumn(-1, Qt::AscendingOrder);
    filesTable->setSortingEnabled(true);
    //filesTable->update();
    processEvents();
}

void MainWindow::removalProgress(int row, const QString& /*path*/, uint64_t /*size*/, bool rmOk, uint64_t nbrDel) {
    _nbrDeleted = nbrDel;
    qDebug() << "removalProgress: row" << row << "rmOk" << rmOk << "_nbrDeleted" << _nbrDeleted;
    const auto elapsed = progressTimer.elapsed();
    const auto diff = elapsed - prevProgress;
    if (diff >= 1'000) {  // msec
        prevProgress = elapsed;
        const QString text = rmOk ? QString("Removed %1 items").arg(_nbrDeleted) :
                                    QString("Failed to remove some items. Removed %1 items").arg(_nbrDeleted);
        filesFoundLabel->setText(text);
    }
    //itemRemoved(row, 1, size, nbrDeleted);
    rowsToRemove_.insert({ row, rmOk });
}

void MainWindow::removalComplete(bool success) {
    removeRows(); // files that failed to delete will not be removed from the table
    const QString prefix = "COMPLETED | ";
    QString suffix = success ? "DELETE SUCCESSFUL" : "SOME FAILED to DELETE";
    const auto maxDepth = unlimSubDirDepthBtn->isChecked() ? -1 : _maxSubDirDepth;
    const auto filesStr = (maxDepth < 0) ? " files/folders" : " files";
    if (_nbrDeleted == 0) {
        suffix = "NO ITEMS DELETED (check the 'Max subfolder depth' setting)";
    }
    else {
        suffix += ", deleted " + QString::number(_nbrDeleted) + filesStr;
    }
    opEnd = steady_clock::now();
    const auto text = prefix + suffix + ", took " + getElapsedTimeStr();
    filesFoundLabel->setText(text);
    qDebug() << text;
    stopAllThreads();
    setStopped(true);
}

void MainWindow::deepRemoveFilesOnThread_Frv2(const IntQStringMap& rowPathMap)
{
    _removal = true;
    removerFrv2 = std::make_shared<Frv2::FileRemover>(this);
    auto msg = "Removing files and folders...";
    filesFoundLabel->setText(msg);
    qDebug() << msg;

    // NOTE: QMetaObject::invokeMethod with Qt::QueuedConnection
    //  is done in Frv2::FileRemover::removeFilesAndFolders02()

    // DO IT NOW
    removerFrv2->removeFilesAndFolders02(
        rowPathMap,
        // Progress callback
        [this](int row, const QString& path, uint64_t size, bool rmOk, uint64_t nbrDel) {
            removalProgress(row, path, size, rmOk, nbrDel);
        },
        // Completion callback
        [this](bool success) {
            removalComplete(success);
        }
    );
}

void MainWindow::deepRemoveFilesOnThread_Frv3(const IntQStringMap& rowPathMap)
{
    _removal = true;
    removerFrv3 = std::make_shared<Frv3::FileRemover>();
    auto msg = "Removing files and folders...";
    filesFoundLabel->setText(msg);
    qDebug() << msg;

    // Set up progress callback (can update UI)
    removerFrv3->setProgressCallback([this](int row, const QString& path, uint64_t size, bool rmOk, uint64_t nbrDel)
    {
        // Since this callback runs in a different thread, use Qt::QueuedConnection
        QMetaObject::invokeMethod(this, [this, row, path, size, rmOk, nbrDel]() {
            removalProgress(row, path, size, rmOk, nbrDel);
        }, Qt::QueuedConnection);
    });
    removerFrv3->setCompletionCallback([this](bool success)
    {
        // Same as above: different thread, Qt::QueuedConnection
        QMetaObject::invokeMethod(this, [this, success]() {
            removalComplete(success);
        }, Qt::QueuedConnection);
    });

    // DO IT NOW
    removerFrv3->removeFiles(rowPathMap);
}
}
