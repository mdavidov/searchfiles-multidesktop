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
#include "util.h"

#include <QObject>
#include <QtGui>
#include <QtWidgets>

namespace Devonline
{

    ///////////////////////////////////////////////////////////////////////////////////////////////

    QString sizeToHumanReadable(quint64 size)
    {
        try
        {
            if (size == 0) {
                return QString("0 Bytes");
            }
            static const quint64 KILO = 1024;
            static const quint64 MEGA = 1024 * KILO;
            static const quint64 GIGA = 1024 * MEGA;
            static const quint64 TERA = 1024 * GIGA;
            static const quint64 PETA = 1024 * TERA;
            double sz = (double) size;
            QString unit = "Bytes";

            if      (size >= PETA) {
                sz /= (double) PETA;
                unit = "PB";
            }
            else if (size >= TERA) {
                sz /= (double) TERA;
                unit = "TB";
            }
            else if (size >= GIGA) {
                sz /= (double) GIGA;
                unit = "GB";
            }
            else if (size >= MEGA) {
                sz /= (double) MEGA;
                unit = "MB";
            }
            else if (size >= KILO) {
                sz /= (double) KILO;
                unit = "KB";
            }

            int precision = 3;
            if (unit.startsWith("B")) {
                precision = 0;
            }
            else if (sz >= 100) {
                precision = 1;
            }
            else if (sz >= 10) {
                precision = 2;
            }
            else {
                precision = 3;
            }
            return QString("%1 %2").arg(sz, 1, 'f', precision).arg(unit);
        }
        catch (...)
        {
            indicateErrorDbg("Exception");
            return QString("");
        }
    }

    QString elapsedTimeToStr( qint64 elapsedMilSec)
    {
        const qint64 elapsedSec = (elapsedMilSec + 500) / 1000;
        const qint64 hr  =  elapsedSec / 3600;
        const qint64 min = (elapsedSec % 3600) / 60;
        const qint64 sec = (elapsedSec % 60);
        Q_ASSERT( 3600*hr + 60*min + sec == elapsedSec);

        const QString hrStr(  QString( " %1 hr").arg(hr) );
        const QString minStr( QString( " %1 min").arg(min));
        const QString secStr( QString( " %1 sec" ).arg(sec));

        QString hhMmSs;
        if (hr  > 0)
            hhMmSs = hrStr + minStr + secStr;
        else if (min > 0)
            hhMmSs = minStr + secStr;
        else
            hhMmSs = secStr;
        return QString( OvSk_FsOp_ELAPSED_TIME_TXT).arg( hhMmSs);

        // OR:
        //const QTime elapsedTime = QTime(0, 0, 0, 0).addSecs(  elapsedMilSec / 1000)
        //                                           .addMSecs( elapsedMilSec % 1000);
        //return QString( OvSk_FsOp_ELAPSED_TIME_TXT)
        //                .arg( elapsedTime.toString("H:mm:ss.zzz' (hr:min:sec.ms)'"));
    }

    void indicateErrorDbg(const QString& text)
    {
        (void) text;
        #ifndef NDEBUG
            try {
                #if !defined(Q_OS_MAC)
                    QMessageBox::warning( 0, text, text);
                #endif
            }
            catch (...) {
            }
        #else
            (void) text;
        #endif
    }

}
