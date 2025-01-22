#pragma once
#include <QObject>
#include <QEvent>
#include <QKeyEvent>

class KeyPressEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KeyPressEventFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                // Emit the Enter key press event
                emit enterKeyPressed();
                return true; // Event handled
            }
        }
        return QObject::eventFilter(obj, event); // Pass the event on to the parent class
    }

signals:
    void enterKeyPressed();
};
