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

#include "helpdialog.hpp"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>


HelpDialog::HelpDialog(QWidget* parent /*= nullptr*/) : QDialog(parent) {
    setWindowTitle("Help");
    setModal(true);
    resize(600, 420);  // Set a reasonable default size

    QVBoxLayout* layout = new QVBoxLayout(this);

    QTextBrowser* helpBrowser = new QTextBrowser(this);
    helpBrowser->setOpenExternalLinks(true);
    helpBrowser->setHtml(R"(
        <h2>Application Help</h2>
        <p>This app can be used to search large, remote folder structures that are not indexed by the local search indexer.
        <h3>Getting Started</h3>
        <p>Usage tips</p>
        <ul>
            <li>Select the folder you want to search using the controls in the top left of the window. Either click the “Browse…” button and navigate through the file system. Or type the path to the folder in the file-system drop-down box to the right of the “Search folder {SF}:” text. Start by typing the beginning of the path (e.g. C:\Users, or /Users) and the drop-down control will list available folders in that location).</li>
            <li>When a desired folder is selected, click the “Search” button in the bottom right of the window.</li>
            <li>Before the search has completed it can be stopped by clicking the Stop button or pressing the Escape (esc) key.</li>
            <li>When the search is done (it's either completed or stopped/interrupted) you can select some files and folders if you want to delete them, and click the “Delete” button. IMPORTANT: Deleted files and folders will NOT be moved to the Recycle Bin / Bin, they will be PERMANENTLY DELETED!</li>
            <li>When “Max subfolder depth” is limited to 0, only the contents of the selected folder will be searched and shown. When it’s limited to 1, only the first level of subfolders (and the selected folder) will be searched. And so on.</li>
        </ul>
    )");

    QPushButton* closeButton = new QPushButton("Close", this);

    layout->addWidget(helpBrowser);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}
