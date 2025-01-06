// aboutdialog.h
#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("About");
        setModal(true);

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Add application icon/logo if you have one
        QLabel* titleLabel = new QLabel("My Application v1.0");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

        QLabel* descLabel = new QLabel("Description of your application.\n"
            "Copyright © 2024 Your Company\n"
            "All rights reserved.");

        QPushButton* closeButton = new QPushButton("Close");

        layout->addWidget(titleLabel);
        layout->addWidget(descLabel);
        layout->addWidget(closeButton);

        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    }
};
