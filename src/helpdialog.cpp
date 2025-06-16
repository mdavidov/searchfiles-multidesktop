#include "precompiled.h"
#include "helpdialog.h"
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>


HelpDialog::HelpDialog(QWidget* parent /*= nullptr*/) : QDialog(parent) {
    setWindowTitle("Help");
    setModal(true);
    resize(600, 420);  // Set a reasonable default size

    QVBoxLayout* layout = new QVBoxLayout(this);

    QTextBrowser* helpBrowser = new QTextBrowser;
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

    QPushButton* closeButton = new QPushButton("Close");

    layout->addWidget(helpBrowser);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}
