#pragma once
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

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

namespace Devonline
{
    class EventFilter : public QObject
    {
        Q_OBJECT

    public:
        explicit EventFilter(QObject* parent = nullptr) : QObject(parent) {}

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyRelease) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Enter|| keyEvent->key() == Qt::Key_Return) {
                    // Emit the Enter key press event
                    emit enterKeyReleased();
                    return true; // Event handled
                }
                else if (keyEvent->key() == Qt::Key_Escape) {
                    emit escapeKeyReleased();
                    return true; // Event handled
                }
            }
            return QObject::eventFilter(obj, event); // Pass the event on to the parent class
        }

    signals:
        void enterKeyReleased();
        void escapeKeyReleased();
    };
}

Q_DECLARE_METATYPE(Devonline::EventFilter)

