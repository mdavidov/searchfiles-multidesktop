#pragma once
// helpdialog.h

#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    HelpDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Help");
        setModal(true);
        resize(600, 400);  // Set a reasonable default size

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
};
