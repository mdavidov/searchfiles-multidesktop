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

#include "config.h"
#include <QtGlobal>
#include <QString>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace mmd
{
    /// @brief Formats the string with easily readable value
    /// of @p size in B, KiB, MiB, GiB, or TiB (depending on the size).
    QString sizeToHumanReadable(quint64 size);

    /// @brief Converts elapsed time in milliseconds to a human-readable string.
    /// The string is formatted as "X hr Y min Z sec" or "Y min Z sec" or "Z sec".
    QString elapsedTimeToStr(qint64 elapsedMilSec);

    /// @brief In Debug build on Mac it pops up a message box
    /// with the @p text.
    void indicateErrorDbg(const QString& text);
}
