#pragma once

#include <QWidgets/QDialog>
class QWidget;

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};
