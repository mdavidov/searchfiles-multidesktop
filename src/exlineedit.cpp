/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "exlineedit.h"
#include "searchlineedit.h"

#include <QtCore/QEvent>
#include <QtCore/QMimeData>
#include <QtCore/QDebug>

#include <QtGui/QDrag>
#include <QtGui/QFocusEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionFrame>


ExLineEdit::ExLineEdit(QWidget *parent)
    : QWidget(parent)
    , m_leftWidget(0)
    , m_lineEdit(new QLineEdit(this))
    , m_clearButton(0)
{
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(m_lineEdit->sizePolicy());
    setBackgroundRole(m_lineEdit->backgroundRole());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    QPalette p = m_lineEdit->palette();
    setPalette(p);

    // line edit
    m_lineEdit->setFrame(false);
    m_lineEdit->setFocusProxy(this);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    QPalette clearPalette = m_lineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    m_lineEdit->setPalette(clearPalette);

    // clearButton
    m_clearButton = new ClearButton(this);
    connect(m_clearButton, SIGNAL(clicked()),
            m_lineEdit, SLOT(clear()));
    connect(m_lineEdit, SIGNAL(textChanged(QString)),
            m_clearButton, SLOT(textChanged(QString)));
}

void ExLineEdit::setLeftWidget(QWidget *widget)
{
    m_leftWidget = widget;
}

QWidget *ExLineEdit::leftWidget() const
{
    return m_leftWidget;
}

void ExLineEdit::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(m_leftWidget);
    updateGeometries();
    QWidget::resizeEvent(event);
}

void ExLineEdit::updateGeometries()
{
    QStyleOptionFrame panel;
    initStyleOption(&panel);
    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int height = rect.height();
    int width = rect.width();

    int m_leftWidgetHeight = m_leftWidget->height();
    m_leftWidget->setGeometry(rect.x() + 2,          rect.y() + (height - m_leftWidgetHeight)/2,
                              m_leftWidget->width(), m_leftWidget->height());

    int clearButtonWidth = this->height();
    m_lineEdit->setGeometry(m_leftWidget->x() + m_leftWidget->width(),        0,
                            width - clearButtonWidth - m_leftWidget->width(), this->height());

    m_clearButton->setGeometry(this->width() - clearButtonWidth, 0,
                               clearButtonWidth, this->height());
}

void ExLineEdit::initStyleOption(QStyleOptionFrame *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (m_lineEdit->isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrame::None;
}

QSize ExLineEdit::sizeHint() const
{
    m_lineEdit->setFrame(true);
    QSize size = m_lineEdit->sizeHint();
    m_lineEdit->setFrame(false);
    return size;
}

void ExLineEdit::focusInEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);
    QWidget::focusInEvent(event);
}

void ExLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);

    if (m_lineEdit->completer()) {
        connect(m_lineEdit->completer(), SIGNAL(activated(QString)),
                         m_lineEdit, SLOT(setText(QString)));
        connect(m_lineEdit->completer(), SIGNAL(highlighted(QString)),
                         m_lineEdit, SLOT(_q_completionHighlighted(QString)));
    }
    QWidget::focusOutEvent(event);
}

void ExLineEdit::keyPressEvent(QKeyEvent *event)
{
    m_lineEdit->event(event);
}

bool ExLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        return m_lineEdit->event(event);
    return QWidget::event(event);
}

void ExLineEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionFrame panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
}

QVariant ExLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
    return m_lineEdit->inputMethodQuery(property);
}

void ExLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
    m_lineEdit->event(e);
}

