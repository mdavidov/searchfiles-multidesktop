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

#include "helpdialog.h"
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
        <h3>Getting Started</h3>
        <p>Here's how to use the application:</p>
        <ul>
            <li>First step...</li>
            <li>Second step...</li>
        </ul>
        <h3>Features</h3>
        <p>Description of features...</p>
    )");

    QPushButton* closeButton = new QPushButton("Close", this);

    layout->addWidget(helpBrowser);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}
