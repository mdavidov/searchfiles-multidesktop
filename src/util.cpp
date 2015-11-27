/****************************************************************************
**
** Copyright (c) 2010 Milivoj (Mike) Davidov
** All rights reserved.
**
** THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND.
** COPYRIGHT HOLDER FURTHER DISCLAIMS ANY IMPLIED WARRANTIES, INCLUDING, WITHOUT
** LIMITATION, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF THE
** USE OR PERFORMANCE OF THE SOFTWARE REMAINS WITH YOU. SHOULD THE SOFTWARE
** PROVE DEFECTIVE, YOU (AND NOT COPYRIGHT HOLDER) ASSUME THE ENTIRE COST OF ALL
** NECESSARY SERVICING OR REPAIR.
**
** IN NO EVENT SHALL COPYRIGHT HOLDER OR ANYONE ELSE INVOLVED IN THE CREATION,
** PRODUCTION, MARKETING, DISTRIBUTION, OR DELIVERY OF THE SOFTWARE, BE LIABLE
** FOR ANY DAMAGES WHATSOEVER; INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS
** OF BUSINESS PROFITS, FOR BUSINESS INTERRUPTION, FOR LOSS OF BUSINESS
** INFORMATION, OR FOR OTHER MONETARY LOSS, ARISING OUT OF THE USE OF THE
** SOFTWARE OR THE INABILITY TO USE THE SOFTWARE, EVEN IF YOU HAVE BEEN NOTIFIED
** OF THE POSSIBILITY OF SUCH DAMAGES.
**
** IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
** CONSEQUENTIAL DAMAGES, OR FOR ANY DAMAGES WHATSOEVER, WHETHER IN A CONTRACT
** ACTION, NEGLIGENCE OR OTHER TORT ACTION, OR OTHER CLAIM OR ACTION, ARISING
** OUT OF, OR IN CONNECTION WITH, THE USE OR PERFORMANCE OF THE SOFTWARE OR
** DOCUMENTS AND OTHER INFORMATION PROVIDED TO YOU BY COPYRIGHT HOLDER, OR IN THE
** PROVISION OF, OR FAILURE TO PROVIDE, SERVICES OR INFORMATION.
**
** Because some states of the United States and some countries do not allow the
** exclusion or limitation of the liability for consequential or incidental
** damages, the above disclaimer may not apply to you. Any warranties that by
** law survive the foregoing disclaimers shall terminate ninety (90) days from
** the date you downloaded or otherwise received the Software.
**
****************************************************************************/
#if 0
#endif

#include "precompiled.h"
#include "config.h"
#include "util.h"

#include <QObject>
#include <QtGui>
#include <QtWidgets>

namespace Overskys
{

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool sizeToHumanReadable( quint64 size, QString & humanReadable)
    {
        try
        {
            if (size == 0)
                humanReadable = QString("0 Bytes");

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

            humanReadable = QString("%1 %2").arg(sz, 1, 'f', precision).arg(unit);
            return true;
        }
        catch (...)
        {
            indicateErrorDbg("Exception");
            return false;
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

    void indicateErrorDbg(const QString & text)
    {
        #ifndef NDEBUG
            try
            {
                QMessageBox::warning( 0, text, text);
            }
            catch (...)
            {
            }
        #else
            (void) text;
        #endif
    }

}
