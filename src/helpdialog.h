#pragma once

#include <QDialog>
class QWidget;

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    explicit HelpDialog(QWidget* parent = nullptr);
};
