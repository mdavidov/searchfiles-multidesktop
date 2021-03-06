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
#include "precompiled.h"
#include "getinfo.h"
#include "config.h"
#include "util.h"
#include <map>
#include <QDesktopServices>
#include <QtGui>
#include <QtWidgets>
#include <QStandardPaths>

namespace Overskys
{

const QString findText( "&Get Info");
const QString stopText( "S&top");

const int RELPATH_COL_IDX = 0;

#if defined(Q_OS_WIN)
    #define eCod_MIN_PATH_LEN 3
#else
    #define eCod_MIN_PATH_LEN 1
#endif


FindInFilesDlg::FindInFilesDlg( const QString & /*dirPath*/, QWidget * parent)
    : QMainWindow(parent)
    , _ignoreDirPathChange(false)
    , _origDirPath( QDir::toNativeSeparators( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory)))
    , _maxSubDirDepth(0)
    , _unlimSubDirDepth(true)
    , _stopped(true)
{
    setWindowTitle( QString(OvSk_FsOp_APP_NAME_TXT) + " " + OvSk_FsOp_APP_VERSION_STR + "." + OvSk_FsOp_APP_BUILD_NBR_STR); // + " by " + OvSk_FsOp_COPMANY_NAME_TXT);

    const QString savedPath = Cfg::St().value( Cfg::origDirPathKey).toString();
    if (!savedPath.isEmpty())
        _origDirPath = savedPath;
    if (_origDirPath.length() > eCod_MIN_PATH_LEN && _origDirPath.endsWith( QDir::separator()))
        _origDirPath.chop(1);

    createSubDirLayout();
    createItemTypeCheckLayout();
    createNavigLayout();
    createExclLayout();
    createFilesTable();
    createMainLayout();

    showMoreOptions(false);
    itemSelectionChanged();
    SetDirPath(_origDirPath);

    _completerTimer.setSingleShot(true);
    bool c = connect(&_completerTimer, SIGNAL(timeout()), this, SLOT(completerTimeout())); Q_ASSERT(c);
    _editTextTimeDiff.restart();

    c = connect(filesCheck,    SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;
    c = connect(foldersCheck,  SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;
    c = connect(symlinksCheck, SIGNAL(stateChanged(int)), this, SLOT(scopeCheckClicked(int))); Q_ASSERT(c); (void)c;

    createRightClickMenu();

    findButton->setDefault(true);
    findButton->setFocus();
    setStopped(true);
}

void FindInFilesDlg::createSubDirLayout()
{
    maxSubDirDepthLbl = new QLabel(tr("Max Subfolder Depth: "));
    setAllTips(maxSubDirDepthLbl, tr("Specify unlimited or maximum sub-folder depth."));
    unlimSubDirDepthBtn = new QRadioButton(tr("Unlimited "));
    unlimSubDirDepthBtn->setChecked(true);
    limSubDirDepthBtn = new QRadioButton(tr("Limited To: "));
    maxSubDirDepthEdt = new QLineEdit();
    maxSubDirDepthEdt->setEnabled(false);
    maxSubDirDepthEdt->setFixedWidth(60);
    QIntValidator * intValidator = new QIntValidator();
    intValidator->setBottom(0);
    maxSubDirDepthEdt->setValidator(intValidator);
    QButtonGroup* subDirDepthGrp = new QButtonGroup(this);
    subDirDepthGrp->setExclusive(true);
    subDirDepthGrp->addButton(unlimSubDirDepthBtn, 0);
    subDirDepthGrp->addButton(limSubDirDepthBtn,   1);

    subDirDepthLout = new QHBoxLayout();
    subDirDepthLout->addWidget( maxSubDirDepthLbl);
    subDirDepthLout->addWidget( unlimSubDirDepthBtn);
    subDirDepthLout->addWidget( limSubDirDepthBtn);
    subDirDepthLout->addWidget( maxSubDirDepthEdt);
    subDirDepthLout->addStretch();
    bool c = connect(unlimSubDirDepthBtn, &QRadioButton::toggled, this, &FindInFilesDlg::unlimSubDirDepthToggled); Q_ASSERT(c); (void)c;
}

void FindInFilesDlg::createItemTypeCheckLayout()
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

void FindInFilesDlg::createNavigLayout()
{
    goUpButton = new QToolButton();
    goUpButton->setText(tr("Up"));
    setAllTips(goUpButton, "Go up in the folder hierarchy and set it as the search folder. ");
    bool c = connect(goUpButton, SIGNAL(clicked()), this, SLOT(goUpBtnClicked())); Q_ASSERT(c);

    browseButton = new QToolButton();
    browseButton->setText(tr("..."));
    setAllTips(browseButton, eCod_BROWSE_FOLDERS_TIP);
    c = connect(browseButton, SIGNAL(clicked()), this, SLOT(browseBtnClicked())); Q_ASSERT(c);

    navigLout = new QHBoxLayout();
    navigLout->addWidget( goUpButton);
    navigLout->addWidget( browseButton);
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
    wordsLineEdit->setPlaceholderText("Search Words");
    setAllTips(wordsLineEdit, OvSk_FsOp_CONTAINING_TEXT_TIP);
    modifyFont(wordsLineEdit, +0.0, true, false, false);
    matchCaseCheck = new QCheckBox(tr("&Match Case"));
    setAllTips(matchCaseCheck, "Match or ignore the case of letters in search words. Does not affect file/folder names. ");

    wordsLout = new QHBoxLayout();
    wordsLout->addWidget(wordsLineEdit);
    wordsLout->addWidget(matchCaseCheck);

    namesLineEdit = new QLineEdit(); //createComboBoxText();
    namesLineEdit->setPlaceholderText("File/Folder Names");
    setAllTips(namesLineEdit, OvSk_FsOp_NAME_FILTERS_TIP);
    modifyFont(namesLineEdit, +0.0, true, false, false);
}

void FindInFilesDlg::createExclLayout()
{
    toggleExclBtn = new QToolButton();
    toggleExclBtn->setText(tr("-"));
    setAllTips(toggleExclBtn, eCod_SHOW_EXCL_OPTS_TIP);
    bool c = connect(toggleExclBtn, SIGNAL(clicked()), this, SLOT(toggleExclClicked())); Q_ASSERT(c); (void)c;

    exclFilesByTextCombo = new QLineEdit(); //createComboBoxText();
    exclFilesByTextCombo->setPlaceholderText("Exclude Files with Words");
    setAllTips(exclFilesByTextCombo, eCod_EXCL_FILES_BY_CONTENT_TIP);
    modifyFont(exclFilesByTextCombo, +0.0, false, false, false);

    exclByFileNameCombo = new QLineEdit(); //createComboBoxText();
    exclByFileNameCombo->setPlaceholderText("Exclude by Partial File Names");
    setAllTips(exclByFileNameCombo, eCod_EXCL_FILES_BY_NAME_TIP);
    modifyFont(exclByFileNameCombo, +0.0, false, false, false);

    exclByFolderNameCombo = new QLineEdit(); //createComboBoxText();
    exclByFolderNameCombo->setPlaceholderText("Exclude by Partial Folder Names");
    setAllTips(exclByFolderNameCombo, eCod_EXCL_FOLDERS_BY_NAME_TIP);
    modifyFont(exclByFolderNameCombo, +0.0, false, false, false);

    exclHiddenCheck = new QCheckBox();
    exclHiddenCheck->setChecked(false);
    exclHiddenCheck->setText("Exclude Hidden");
    setAllTips(exclHiddenCheck, eCod_EXCL_HIDDEN_ITEMS);
    modifyFont(exclHiddenCheck, +0.0, false, false, false);
}

void FindInFilesDlg::createMainLayout()
{
    filesFoundLabel = new QLabel;
    modifyFont(filesFoundLabel, +0.0, true, false, false);

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

    mainLayout->addWidget(dirComboBox,      gridRowIdx, 0, 1, 3);
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
    resize(920, 410);
}

void FindInFilesDlg::modifyFont(QWidget * widget, qreal ptSzDelta, bool bold, bool italic, bool underline)
{
    QFont font(widget->font());

    qDebug() << "Font orig: " << font.toString();
    font.setPointSizeF(font.pointSizeF() + ptSzDelta);
    font.setBold(bold);
    font.setItalic(italic);
    font.setUnderline(underline);
    qDebug() << "Font new:  " << font.toString();

    widget->setFont(font);
}

void FindInFilesDlg::setAllTips(QWidget * widget, const QString & text)
{
    widget->setToolTip(  text);
    widget->setStatusTip(text); // does not work in dlg based app
    widget->setWhatsThis(text);
}

void FindInFilesDlg::scopeCheckClicked(int /*newCheckState*/)
{
    const bool filesChecked = (filesCheck->checkState() == Qt::Checked);
    wordsLineEdit->setEnabled(filesChecked);
    matchCaseCheck->setEnabled(filesChecked);
    exclByFileNameCombo->setEnabled(filesChecked);
    exclFilesByTextCombo->setEnabled(filesChecked);
}

void FindInFilesDlg::goUpBtnClicked()
{
    const QString dirPath = dirComboBox->currentText();
    if (dirPath.isEmpty())
        return;
    QDir dir(dirPath);

    const bool ok = dir.cdUp();
    if (ok)
        SetDirPath( QDir::toNativeSeparators( dir.absolutePath()));
    else if (dirPath.length() <= eCod_MIN_PATH_LEN)
        SetDirPath( "");
}

void FindInFilesDlg::browseBtnClicked()
{
    const QString selDir = QFileDialog::getExistingDirectory(this, tr("Select Folder"), dirComboBox->currentText());
    if (!selDir.isEmpty())
        SetDirPath(selDir);
}

void FindInFilesDlg::showMoreOptions(bool show)
{
    toggleExclBtn->setText(show ? "-" : "+");
    toggleExclBtn->setToolTip(show ? eCod_SHOW_EXCL_OPTS_TIP : eCod_HIDE_EXCL_OPTS_TIP);
    exclByFolderNameCombo->setVisible(show);
    exclByFileNameCombo->setVisible(show);
    exclFilesByTextCombo->setVisible(show);
    exclHiddenCheck->setVisible(show);
}

void FindInFilesDlg::toggleExclClicked()
{
    const bool show = toggleExclBtn->text().startsWith("+");
    const int deltaH = 100; // TODO calc the height of excl. widgets
    showMoreOptions(show);
    resize(width(), height() + (show ? +deltaH : -deltaH));
}

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

void FindInFilesDlg::GetFileInfos(QFileInfoList & fileInfos) const
{
    const QList<QTableWidgetItem *>  selectedItems = filesTable->selectedItems();
    foreach (const QTableWidgetItem * item, selectedItems) {
        if (filesTable->column( item) == RELPATH_COL_IDX)
            fileInfos.append( QFileInfo( item->data(Qt::UserRole).toString() )); //( item->text()));
    }
}

/////////////////////////////////////////////////

void FindInFilesDlg::deleteBtnClicked()
{
    try {
        _opType = Overskys::Op::deletePerm;
        if (filesTable->selectedItems().isEmpty()) {
            QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
            return;
        }

        setStopped(false);
        _elTimer.restart();
        qApp->processEvents();

        std::map< int, QString, bigger<int> > itemList;

        getSelectedItems(itemList); // GET SELECTED ITEMS
        const int nonSelectCount = filesTable->rowCount() - filesTable->selectedItems().count();

        removeItems(itemList); // REMOVE ITEMS

        emit filesTable->itemSelectionChanged();
        qApp->processEvents();
        setStopped(true);

        if (filesTable->rowCount() != nonSelectCount) {
            if (nonSelectCount <= 0)
                filesTable->clear();
            else
                findBtnClicked();
        }
        else
            setFilesFoundLabel();
    }
    catch (...) { Q_ASSERT(false); } // TODO tell the user
    setStopped(true);
}

void FindInFilesDlg::getSelectedItems(std::map<int, QString, bigger<int>> & itemList)
{
    const QList<QTableWidgetItem *>  selectedItems = filesTable->selectedItems();
    foreach (const QTableWidgetItem * item, selectedItems)
    {
        if (filesTable->column( item) == RELPATH_COL_IDX)
        {
            if (_stopped)
                break;
            const QString path = item->data(Qt::UserRole).toString(); //item->text();
            const int row = filesTable->row(item);
            Q_ASSERT(itemList.find(row) == itemList.end());
            if (itemList.find(row) == itemList.end())
                itemList.insert(std::pair<int, QString>(row, path));
            if (isTimeToReport())
                qApp->processEvents();
        }
    }
}
void FindInFilesDlg::removeItems(const std::map<int, QString, bigger<int>>& itemList)
{
    int delCnt = 0;
    for (std::map<int, QString, bigger<int> >::const_iterator it = itemList.cbegin(); it != itemList.cend(); ++it)
    {
        try {
            if (_stopped) break;
            const QString path = it->second;

            if (!QFileInfo(path).isDir()) {
                // RM FILE
                QFile file( it->second);
                const bool rmok = file.remove();
                if (rmok) {
                    filesTable->removeRow( it->first);
                    ++delCnt;
                    if ((delCnt % 5) == 0)
                        emit filesTable->itemSelectionChanged();
                }
            }
            else {
                // RM DIR
                QDir dir( path);
                bool rmok = dir.rmdir( path);
                if (!rmok)
                     rmok = dir.removeRecursively();
                if ( rmok) {
                    filesTable->removeRow( it->first);
                    ++delCnt;
                    if ((delCnt % 5) == 0)
                        emit filesTable->itemSelectionChanged();
                }
            }
            if (isTimeToReport())
                qApp->processEvents();
        }
        catch (...) { Q_ASSERT(false); } // TODO tell the user
    }
}

void FindInFilesDlg::shredBtnClicked()
{
    try
    {
        _opType = Overskys::Op::shredPerm;
        setStopped(true);
        if (filesTable->selectedItems().isEmpty()) {
            QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_FOUNDFILES_TXT);
            return;
        }

        // TODO - IMPLEMENT Shredding

        emit filesTable->itemSelectionChanged();
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::cancelBtnClicked()
{
    setStopped(true);
    //reject();
}

void FindInFilesDlg::SetDirPath( const QString & dirPath)
{
    try
    {
        _origDirPath = QDir::toNativeSeparators( dirPath);
        if (dirComboBox->findText(_origDirPath) == -1)
            dirComboBox->addItem(_origDirPath);
        dirComboBox->setCurrentIndex( dirComboBox->findText(_origDirPath));
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::Clear()
{
    try
    {
        filesTable->setRowCount(0);
        filesTable->hide();
        filesTable->show();
        filesFoundLabel->setText("");
        _outFiles.clear();
        _dirCount = 0;
        _totItemCount = 0;
        _totItemsSize = 0;
        _foundItemsSize = 0;
        _elTimer.restart();
        _prevEl = 0;
        setStopped(true);
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::setStopped(bool stopped)
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

void FindInFilesDlg::setFilesFoundLabel(const QString & prefix/*= QString()*/)
{
    QString totItemsSizeStr;
    sizeToHumanReadable(_totItemsSize, totItemsSizeStr);
    QString foundItemsSizeStr;
    sizeToHumanReadable(_foundItemsSize, foundItemsSizeStr);
    if (filesTable->rowCount() == 0) {
        filesFoundLabel->setText( prefix + tr("Found no items (searched %1 items, combined size %2).")
            .arg(_totItemCount).arg(totItemsSizeStr));
    }
    else if (filesTable->rowCount() == 1) {
        filesFoundLabel->setText( prefix + tr("Found 1 item, size %1 (searched %2 items, combined size %3). ")
            .arg(foundItemsSizeStr).arg(_totItemCount).arg(totItemsSizeStr));
    }
    else if (filesTable->rowCount() > 1) {
        filesFoundLabel->setText( prefix + tr("Found %1 items, combined size %2 (searched %3 items, combined size %4). ")
            .arg(filesTable->rowCount()).arg(foundItemsSizeStr).arg(_totItemCount).arg(totItemsSizeStr));
    }
}

void FindInFilesDlg::findFilesPrep()
{
    _fileNameFilter = namesLineEdit->text();
    updateComboBox( dirComboBox);

    // GET FILTERS
    if (_fileNameFilter.isEmpty())
        _fileNameFilter = "*";
    const QString tmpFilter = _fileNameFilter.replace( QRegExp("[ ]*;[ ]*"), ";");
    _fileNameSubfilters = tmpFilter.split(";", QString::SkipEmptyParts);

    _itemTypeFilter = QDir::Filters( QDir::Drives | QDir::System | QDir::NoDotAndDotDot);
    if (filesCheck->isChecked()   || symlinksCheck->isChecked())
        _itemTypeFilter |= QDir::Files;
    if (foldersCheck->isChecked() || symlinksCheck->isChecked())
        _itemTypeFilter |= QDir::Dirs;
    if (!symlinksCheck->isChecked())
        _itemTypeFilter |= QDir::NoSymLinks;
    if (!exclHiddenCheck->isChecked())
        _itemTypeFilter |= QDir::Hidden;

    _matchCase = matchCaseCheck->isChecked();
    _origDirPath = QDir::toNativeSeparators( dirComboBox->currentText());
    while (_origDirPath.length() > eCod_MIN_PATH_LEN && _origDirPath.endsWith( QDir::separator()))
        _origDirPath.chop(1);

    _searchWords        = wordsLineEdit->text().split(" ", QString::SkipEmptyParts);;
    _exclusionWords     = exclFilesByTextCombo->text().split(" ", QString::SkipEmptyParts);
    _exclFilePatterns   = GetSimpleNamePatterns( exclByFileNameCombo->text());
    _exclFolderPatterns = GetSimpleNamePatterns( exclByFolderNameCombo->text());

    bool maxValid = false;
    _maxSubDirDepth =  maxSubDirDepthEdt->text().toInt(&maxValid);
    _unlimSubDirDepth = (!maxValid || unlimSubDirDepthBtn->isChecked());
}

void FindInFilesDlg::findBtnClicked()
{
  try
  {
    if (!_stopped) {
        filesTable->sortByColumn( -1, Qt::AscendingOrder);
        filesTable->setSortingEnabled( true);
        setStopped(true);
        setFilesFoundLabel(tr("Stopped. "));
        return;
    }
    if (!filesCheck->isChecked() && !foldersCheck->isChecked() && !symlinksCheck->isChecked()) {
        filesTable->sortByColumn( -1, Qt::AscendingOrder);
        filesTable->setSortingEnabled( true);
        QMessageBox::warning( this, OvSk_FsOp_APP_NAME_TXT, OvSk_FsOp_SELECT_ITEM_TYPE_TXT);
        return;
    }
    filesTable->setSortingEnabled( false);
    Clear();
    setStopped(false);

    findFilesPrep();

    // PROCESS FILES
    findFilesRecursive( _origDirPath, 0);

    setFilesFoundLabel( _stopped ? "Interrupted. " : "");
    setStopped(true);

    filesTable->sortByColumn( -1, Qt::AscendingOrder);
    filesTable->setSortingEnabled( true);
  }
  catch (...) { Q_ASSERT(false); } // TODO tell the user
}

bool FindInFilesDlg::findItem(const QString & dirPath, const QFileInfo& fileInfo)
{
    try {
        if (isTimeToReport())
            qApp->processEvents();
        const QString filePath = QDir::fromNativeSeparators( fileInfo.absoluteFilePath());
        bool toExclude = fileInfo.isHidden() && exclHiddenCheck->isChecked();
        if (!toExclude && !_exclFolderPatterns.isEmpty()) {
            toExclude = StringContainsAnyWord( dirPath, _exclFolderPatterns);
        }
        if (!toExclude && fileInfo.isFile() && !_exclFilePatterns.isEmpty()) {
            toExclude = StringContainsAnyWord( fileInfo.fileName(), _exclFilePatterns);
        }
        if (!toExclude && fileInfo.isFile() && !_exclusionWords.isEmpty()) {
            toExclude = fileContainsAnyWord( filePath, _exclusionWords);
        }
        if (!toExclude) {
            bool toAppend = false;
            if (fileInfo.isSymLink()) {
                toAppend = symlinksCheck->isChecked();
            }
            if (fileInfo.isDir()) {
                toAppend |= foldersCheck->isChecked();
            }
            if (fileInfo.isFile()) {
                toAppend |= filesCheck->isChecked() && (_searchWords.isEmpty() ||
                            fileContainsAllWords( filePath, _searchWords));
            }
            if (toAppend)
            {
                _outFiles.append( filePath);
                appendFileToTable( filePath, fileInfo);
                _foundItemsSize += fileInfo.size();
            }
        }
        return true;
    }
    catch (...) { Q_ASSERT(false); return false; } // TODO tell the user
}

quint64 FindInFilesDlg::combinedSize(const QFileInfoList& items)
{
    quint64 csz = 0;
    foreach (QFileInfo item, items)
    {
        if (_stopped) return csz;
        csz += item.size();
    }
    return csz;
}

void FindInFilesDlg::findFilesRecursive( const QString & dirPath, qint32 subDirDepth)
{
    // report progress
    ++_dirCount;
    if (isTimeToReport()) {
        qApp->processEvents();
        filesFoundLabel->setText( tr("Examined %1 items in %2 folders so far...").arg(_totItemCount).arg(_dirCount));
    }

    // find all items by name filters
    QDir currentDir = QDir( dirPath);
    currentDir.setNameFilters( _fileNameSubfilters);
    currentDir.setFilter( _itemTypeFilter);
    const QFileInfoList fileInfos = currentDir.entryInfoList();
    QDir unFilteredDir = QDir( dirPath);
    static const QDir::Filters unFilter = QDir::AllEntries | QDir::Drives | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot;
    const QFileInfoList unFilteredItems = unFilteredDir.entryInfoList(unFilter);
    _totItemCount += unFilteredItems.count();
    _totItemsSize += combinedSize(unFilteredItems);
    foreach (QFileInfo fileInfo, fileInfos)
    {
        if (_stopped) return;
        findItem(dirPath, fileInfo);
    }

    // recurse into each subdir
    if (_unlimSubDirDepth || subDirDepth < _maxSubDirDepth || _maxSubDirDepth < 0)
    {
        const QDir curDir = QDir( dirPath);
        QDir::Filters filters = QDir::Dirs | QDir::Drives | QDir::System | QDir::NoDotAndDotDot;
        if (!exclHiddenCheck->isChecked())
            filters |= QDir::Hidden;
        QStringList subDirs = curDir.entryList( filters);
        foreach (QString subDir, subDirs)
        {
            if (_stopped) return;
            const QString absSubDirPath = curDir.absoluteFilePath( subDir);
            const QFileInfo subDirInfo( absSubDirPath);
            if (subDirInfo.isDir() && !subDirInfo.isSymLink()) // do not follow symlinks
                findFilesRecursive( absSubDirPath, subDirDepth + 1);
        }
    }
}

QStringList FindInFilesDlg::GetSimpleNamePatterns( const QString & rawNamePatters) const
{
    QStringList outPatters;
    const QStringList tempPatterns = rawNamePatters.split(";", QString::SkipEmptyParts);
    foreach (QString tempPat, tempPatterns)
    {
        QString trimmedPat = tempPat.trimmed();
        QString & procPat = trimmedPat.remove(QChar('*'), Qt::CaseInsensitive);
        outPatters.append( procPat);
    }
    return outPatters;
    return outPatters;
}

bool FindInFilesDlg::StringContainsAnyWord(const QString & theString, const QStringList & wordList) const
{
    foreach (QString word, wordList)
    {
        if (theString.contains(word, Qt::CaseInsensitive))
            return true;
    }
    return false;
    return false;
}

bool FindInFilesDlg::fileContainsAllWords( const QString & filePath, const QStringList & wordList)
{
    QFile file( filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    foreach (QString word, wordList)
    {
        if (!fileContainsWord( file, word))
            return false;
    }
    return true;
}

bool FindInFilesDlg::fileContainsWord( QFile & file, const QString & word)
{
    file.seek(0);
    QString line;
    QTextStream stream(&file);

    while (!stream.atEnd())
    {
        if (isTimeToReport())
            qApp->processEvents();

        if (_stopped)
            return false;

        line = stream.readLine();
        if (line.contains( word, _matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive))
            return true;
    }
    return false;
}

bool FindInFilesDlg::fileContainsAnyWord( const QString & filePath, const QStringList & wordList)
{
    try
    {
        QFile file( filePath);
        if (!file.open(QIODevice::ReadOnly))
            return false;

        return fileContainsAnyWord( file, wordList);
    }
    catch (...) { Q_ASSERT(false); } // TODO tell the user
    return true;
}

bool FindInFilesDlg::fileContainsAnyWord( QFile & file,  const QStringList & wordList)
{
    file.seek(0);
    QString line;
    QTextStream stream(&file);

    while (!stream.atEnd())
    {
        if (isTimeToReport())
            qApp->processEvents();

        if (_stopped)
            return false;

        line = stream.readLine();

        foreach (QString word, wordList)
        {
            if (line.contains( word, _matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive))
                return true;
        }
    }
    return false;
}

QStringList FindInFilesDlg::findTextInFiles( const QStringList & files, const QString & textToFind)
{
    if (files.isEmpty()) {
        setFilesFoundLabel();
        return QStringList();
    }

    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i)
    {
        if (_stopped) {
            break;
        }
        if (isTimeToReport()) {
            qApp->processEvents();
            filesFoundLabel->setText( tr("Getting info, file %1 of %2...").arg(i).arg(files.count()));
        }

        QFile file( files[i]);

        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);

            while (!in.atEnd())
            {
                if (_stopped) {
                    break;
                }
                if (isTimeToReport())
                    qApp->processEvents();

                line = in.readLine();
                if (line.contains( textToFind, _matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
                    foundFiles << files[i];
                    appendFileToTable( files[i], QFileInfo( files[i]));
                    break;
                }
            }
        }
    }
    qApp->processEvents();
    setFilesFoundLabel();
    return foundFiles;
}

void FindInFilesDlg::showFiles(const QStringList & files)
{
    if (files.isEmpty()) {
        setFilesFoundLabel();
        return;
    }

    for (int i = 0; i < files.count(); ++i)
    {
        if (_stopped) {
            break;
        }
        if (isTimeToReport()) {
            qApp->processEvents();
            filesFoundLabel->setText( tr("Adding item %1 of %2...").arg(i).arg(files.count()));
        }

        appendFileToTable( files[i], QFileInfo( files[i]));
    }
    qApp->processEvents();
    setFilesFoundLabel();
}

inline bool FindInFilesDlg::isTimeToReport()
{
    if ((_elTimer.elapsed() - _prevEl) >= 100) {
        _prevEl = _elTimer.elapsed();
        return true;
    }
    else
        return false;
}

QPushButton * FindInFilesDlg::createButton(const QString & text, const char *member)
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

void FindInFilesDlg::dirPathEditTextChanged(const QString & newText)
{
    try
    {
        if (_ignoreDirPathChange)
        {
            _ignoreDirPathChange = false;
            return;
        }

        const qint64 timeDiff = _editTextTimeDiff.elapsed();
        qDebug() << "timeDiff:" << timeDiff;

        if (newText.endsWith(QDir::separator()))
        {
            fileSystemModel->setRootPath(newText);
            dirComboBox->completer()->setCompletionPrefix(newText);
            #if defined(Q_OS_WIN)
                if (newText.length() == 1)
                {
                    _ignoreDirPathChange = true;
                    fileSystemModel->setRootPath(eCod_TOP_ROOT_PATH);
                    dirComboBox->completer()->setCompletionPrefix(eCod_TOP_ROOT_PATH);
                    dirComboBox->setCurrentText( eCod_TOP_ROOT_PATH);
                }
            #endif
            _completerTimer.start(33);
        }
        else if (newText.isEmpty())
        {
            if (timeDiff < 40 || timeDiff > 120)
            {
                fileSystemModel->setRootPath("");
                dirComboBox->completer()->setCompletionPrefix("");
                _completerTimer.start(33);
            }
        }
        else // newText is not empty
        {
            if (dirComboBox->completer()->completionPrefix().isEmpty() && timeDiff < 30)
            {
                dirComboBox->completer()->setCompletionPrefix(newText);
            }
        }
        qDebug() << "newText:            " << newText;
        qDebug() << "filesys root path:  " << fileSystemModel->rootPath();
        qDebug() << "completion prefix:  " << dirComboBox->completer()->completionPrefix();

        Cfg::St().setValue( Cfg::origDirPathKey,  QDir::toNativeSeparators(newText));

        _editTextTimeDiff.restart();
    }
    catch(...) { Q_ASSERT(false); }
}

void FindInFilesDlg::completerTimeout()
{
    try
    {
        if (dirComboBox->completer())
            dirComboBox->completer()->complete(); // shows the popup if the completion count > 0
    }
    catch(...) { Q_ASSERT(false); }
}

QFileSystemModel * FindInFilesDlg::newFileSystemModel(QCompleter * completer, const QString & currentDir)
{
    fileSystemModel = new QFileSystemModel(completer);
    fileSystemModel->setReadOnly( true);
    fileSystemModel->setResolveSymlinks( false);
    fileSystemModel->setFilter( QDir::Dirs | QDir::Drives  | QDir::Hidden  | QDir::System | QDir::NoDotAndDotDot);
    fileSystemModel->setRootPath(currentDir);
    //fileSystemModel->sort(0, Qt::AscendingOrder);
    return fileSystemModel;
}

QComboBox * FindInFilesDlg::createComboBoxFSys(const QString & text, bool setCompleter)
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

QComboBox * FindInFilesDlg::createComboBoxText()
{
    QCompleter * completer = new QCompleter();
    completer->setCaseSensitivity( Qt::CaseSensitive);
    completer->setModelSorting( QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode( QCompleter::InlineCompletion);
    completer->setMaxVisibleItems( 16);
    completer->setWrapAround( false);

    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable( true);
    comboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred);

    completer->setParent( comboBox);
    //completer->setModel( ? );
    comboBox->setCompleter( completer);

    return comboBox;
}

inline QString FsItemType(const QFileInfo & fileInfo)
{
    QString fsType;
    if (fileInfo.isSymLink()) {
        //if (fileInfo.isDir())
        //    fsType = OvSk_FsOp_SYMLINK_TXT " to Folder";
        //else if (fileInfo.isFile())
        //    fsType = OvSk_FsOp_SYMLINK_TXT " to File";
        //else
            fsType = OvSk_FsOp_SYMLINK_TXT;
    }
    else if (fileInfo.isDir())
        fsType = "Folder";
    else if (fileInfo.isFile())
        fsType = "File";
    else
        fsType = "";

    if (fileInfo.isHidden())
        fsType += " - Hidden";

    return fsType;
}

void FindInFilesDlg::appendFileToTable(const QString filePath, const QFileInfo & fileInfo)
{
  try
  {
    TableWidgetItem * fileNameItem = new TableWidgetItem( fileInfo.fileName());
    fileNameItem->setFlags( fileNameItem->flags() ^ Qt::ItemIsEditable);

    const QString fpath = QDir::toNativeSeparators( fileInfo.path());
    const int dlen = QDir::toNativeSeparators( _origDirPath).length();
    QString relPath = (fpath.length() <= dlen) ? "" : fpath.right( fpath.length() - dlen);
    if (relPath.startsWith( QDir::separator()))
        relPath = relPath.right( relPath.length() - 1);
    relPath = "{SF}/" + relPath + (relPath.length() > 0 ? "/" : "");
    TableWidgetItem * filePathItem = new TableWidgetItem( QDir::toNativeSeparators( relPath));
    filePathItem->setFlags( filePathItem->flags() ^ Qt::ItemIsEditable);
    filePathItem->setData( Qt::UserRole, QDir::toNativeSeparators( filePath));

    const QString fileExt = !fileInfo.suffix().isEmpty() ? "." + fileInfo.suffix() : "";
    TableWidgetItem * fileExtItem = new TableWidgetItem( (fileInfo.isDir() && !fileInfo.isSymLink()) ? "" : fileExt);
    fileExtItem->setFlags( fileExtItem->flags() ^ Qt::ItemIsEditable);

    TableWidgetItem * fsiTypeItem = new TableWidgetItem( FsItemType( fileInfo));
    fsiTypeItem->setFlags( fsiTypeItem->flags() ^ Qt::ItemIsEditable);

    //const QString dateModStr = fileInfo.lastModified().toString( Qt::SystemLocaleShortDate); //( Qt::ISODate); ( "yyyy.MM.dd HH:mm:ss");
    QTableWidgetItem * dateModItem = new QTableWidgetItem();
    dateModItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter);
    dateModItem->setFlags( dateModItem->flags() ^ Qt::ItemIsEditable);
    dateModItem->setData( Qt::DisplayRole, fileInfo.lastModified());

    const qint64 sizeKB = (fileInfo.size() + (qint64)512) / (qint64)1024;
    QString sizeText; //( fileInfo.isFile() ? tr("%1 KB").arg(int((fileInfo.size() + 512) / 1024)) : tr(""));
    sizeToHumanReadable( fileInfo.size(), sizeText);
    QTableWidgetItem * sizeItem = new QTableWidgetItem();
    sizeItem->setFlags( sizeItem->flags() ^ Qt::ItemIsEditable);
    sizeItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter);
    sizeItem->setData( Qt::DisplayRole, fileInfo.isFile() ? QVariant(sizeKB)   : QVariant(""));
    sizeItem->setData( Qt::ToolTipRole, fileInfo.isFile() ? QVariant(sizeText) : QVariant(""));

    // Owner is always empty (on Win 7) !!!
    //TableWidgetItem * ownerItem = new TableWidgetItem( fileInfo.owner());
    //ownerItem->setFlags( ownerItem->flags() ^ Qt::ItemIsEditable);

    const int row = filesTable->rowCount();
    filesTable->insertRow(row);

    filesTable->setItem( row, 0, filePathItem);
    filesTable->setItem( row, 1, fileNameItem);
    filesTable->setItem( row, 2, sizeItem);
    filesTable->setItem( row, 3, dateModItem);
    filesTable->setItem( row, 4, fileExtItem);
    filesTable->setItem( row, 5, fsiTypeItem);

    filesTable->setRowHeight(row, 45);

    filesTable->scrollToItem( fileNameItem);
    filesFoundLabel->setText( tr("Found %1 items so far...").arg(row + 1));
  }
  catch(...) { Q_ASSERT(false); }
}

void FindInFilesDlg::createFilesTable()
{
  try
  {
    filesTable = new QTableWidget(0, 6, this);
    filesTable->setParent(this);

    filesTable->setWordWrap(true);
    filesTable->setSelectionBehavior( QAbstractItemView::SelectRows);
    filesTable->setAlternatingRowColors( true);
    filesTable->sortByColumn( -1, Qt::AscendingOrder);
    filesTable->setSortingEnabled( false);
    filesTable->setShowGrid( true);

    filesTable->verticalHeader()->hide();
    filesTable->verticalHeader()->setSectionResizeMode( QHeaderView::Interactive);

    QStringList labels;
    labels << tr("Relative Path") << tr("Name") << tr("Size [KB]") << tr("Date Modified") << tr("Extension") << tr("Type"); // << tr("Owner");
    filesTable->setHorizontalHeaderLabels( labels);
    filesTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive);

#if defined(Q_OS_MAC)
    filesTable->setColumnWidth( 0, 420);
    filesTable->setColumnWidth( 1, 180);
    filesTable->setColumnWidth( 2,  80);
    filesTable->setColumnWidth( 3, 150);
    filesTable->setColumnWidth( 4,  90);
    filesTable->horizontalHeader()->setSectionResizeMode( 5, QHeaderView::Stretch);
#elif defined (Q_OS_WIN)
    filesTable->setColumnWidth( 0, 360);
    filesTable->setColumnWidth( 1, 140);
    filesTable->setColumnWidth( 2,  70);
    filesTable->setColumnWidth( 3, 140);
    filesTable->setColumnWidth( 4,  80);
    filesTable->horizontalHeader()->setSectionResizeMode( 5, QHeaderView::Stretch);
#else
    filesTable->setColumnWidth( 0, 360);
    filesTable->setColumnWidth( 1, 140);
    filesTable->setColumnWidth( 2,  70);
    filesTable->setColumnWidth( 3, 150);
    filesTable->setColumnWidth( 4,  80);
    filesTable->horizontalHeader()->setSectionResizeMode( 5, QHeaderView::Stretch);
#endif

    modifyFont(filesTable, +1.0, true, false, false);

    bool
    c = connect( filesTable, SIGNAL(cellActivated(int,int)),
                 this, SLOT(openFileOfItem(int,int))); Q_ASSERT(c); (void)c;
    c = connect( filesTable, SIGNAL(itemSelectionChanged()),
                 this, SLOT(itemSelectionChanged())); Q_ASSERT(c); (void)c;

    filesTable->setContextMenuPolicy( Qt::CustomContextMenu);
    c = connect(filesTable, SIGNAL(customContextMenuRequested(const QPoint &)),
                this,         SLOT(showContextMenu(const QPoint &)));  Q_ASSERT(c);
  }
  catch(...) { Q_ASSERT(false); }
}

void FindInFilesDlg::openFileOfItem( int row, int /* column */)
{
    QTableWidgetItem * item = filesTable->item(row, RELPATH_COL_IDX);

    QDesktopServices::openUrl( QUrl::fromLocalFile( item->data(Qt::UserRole).toString() )); //( item->text()));
}

void FindInFilesDlg::itemSelectionChanged()
{
    deleteButton->setEnabled(_stopped && filesTable->selectedItems().count() > 0);
    shredButton->setEnabled( _stopped && filesTable->selectedItems().count() > 0);
}

void FindInFilesDlg::createRightClickMenu()
{
    try
    {
        openRunAct = new QAction( QIcon(""), eCod_OPEN_CONT_FOLDER_ACT_TXT, this);
        openRunAct->setStatusTip (eCod_OPEN_CONT_FOLDER_STS_TIP);
        connect( openRunAct, SIGNAL(triggered()), this, SLOT(openRunSlot()));
        openRunAct->setShortcutContext( Qt::WidgetWithChildrenShortcut);
        //openRunAct->setShortcut(         QKeySequence( Qt::Key_F3));
        //shortcuts.append( new QShortcut( QKeySequence( Qt::Key_F3), this, SLOT(openRunSlot()), SLOT(openRunSlot()), Qt::WidgetWithChildrenShortcut));

        copyPathAct = new QAction( QIcon(""), eCod_COPY_PATH_ACT_TXT, this);
        copyPathAct->setStatusTip (eCod_COPY_PATH_STS_TIP);
        connect( copyPathAct, SIGNAL(triggered()), this, SLOT(copyPathSlot()));
        copyPathAct->setShortcutContext( Qt::WidgetWithChildrenShortcut);
        copyPathAct->setEnabled(false);

        propertiesAct = new QAction( QIcon(""), eCod_PROPERTIES_ACT_TXT, this);
        propertiesAct->setStatusTip (eCod_PROPERTIES_STS_TIP);
        connect( propertiesAct, SIGNAL(triggered()), this, SLOT(propertiesSlot()));
        propertiesAct->setShortcutContext( Qt::WidgetWithChildrenShortcut);
        propertiesAct->setEnabled(false);

        rightClickMenu = new QMenu( tr("Context Menu"));
        rightClickMenu->addAction( openRunAct);
        rightClickMenu->addSeparator();
        rightClickMenu->addAction( copyPathAct);
        rightClickMenu->addAction( propertiesAct);
        //rightClickMenu->setDefaultAction( openRunAct);
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::showContextMenu(const QPoint & point)
{
    try
    {
        if (rightClickMenu)
            rightClickMenu->popup( filesTable->mapToGlobal( point) /*, m_parent->RightClickMenu()->defaultAction()*/ );
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::openRunSlot()
{
    try
    {
        const QList<QTableWidgetItem *>  selectedItems = filesTable->selectedItems();
        foreach (const QTableWidgetItem * item, selectedItems)
        {
            if (filesTable->column( item) == RELPATH_COL_IDX)
            {
                const QFileInfo fileInfo = QFileInfo( item->data(Qt::UserRole).toString() ); //( item->text());
                if (fileInfo.isDir())
                {
                    const QUrl url = QUrl::fromLocalFile( item->data(Qt::UserRole).toString() ); //( item->text());
                    QDesktopServices::openUrl( url);
                }
                else
                {
                    const QString dirPath = fileInfo.absolutePath();
                    const QUrl url = QUrl::fromLocalFile( dirPath);
                    QDesktopServices::openUrl( url);
                }
                return;
            }
        }
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::copyPathSlot()
{
    try
    {
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::propertiesSlot()
{
    try
    {
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::keyReleaseEvent( QKeyEvent* ev)
{
    try
    {
        if (ev->key() == Qt::Key_Escape)
        {
            _stopped = true;
            ev->accept();
        }
        else
            QMainWindow::keyReleaseEvent(ev);
    }
    catch (...) { Q_ASSERT(false); }
}

void FindInFilesDlg::unlimSubDirDepthToggled(bool /*checked*/)
{
    try
    {
        _unlimSubDirDepth = unlimSubDirDepthBtn->isChecked();
        if (_unlimSubDirDepth)
        {
            bool maxValid = false;
            _maxSubDirDepth =  maxSubDirDepthEdt->text().toInt(&maxValid);
            maxSubDirDepthEdt->setEnabled(false);
            maxSubDirDepthEdt->setText("");
        }
        else
        {
            maxSubDirDepthEdt->setEnabled(true);
            maxSubDirDepthEdt->setText(QString("%1").arg(_maxSubDirDepth));
        }
    }
    catch (...) { Q_ASSERT(false); }
}

} // namespace Overskys
