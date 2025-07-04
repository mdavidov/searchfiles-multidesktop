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

#include "precompiled.h"
#include "config.h"
#include <QtGlobal>
#include <QString>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace Devonline
{
    /// @brief Formats the string (@p humanReadable) with human readable value of @p size in B, KB, MB, GB, or TB.
    QString sizeToHumanReadable(quint64 size);

    ///
    QString elapsedTimeToStr(qint64 elapsedMilSec);

    ///
    void indicateErrorDbg(const QString & text);
}
