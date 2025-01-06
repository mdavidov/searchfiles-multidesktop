/****************************************************************************
**
** Copyright (c) 2012 Milivoj (Mike) Davidov
** All rights reserved.
**
** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
** EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#pragma once

#include "precompiled.h"
#include "config.h"
#include <QtGlobal>
#include <QString>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace Overskys
{
    /// @brief Formats the string (@p humanReadable) with human readable value of @p size in B, KB, MB, GB, or TB.
    bool sizeToHumanReadable( quint64 size, QString & humanReadable);

    ///
    QString elapsedTimeToStr( qint64 elapsedMilSec);

    ///
    void indicateErrorDbg( const QString & text);
}
