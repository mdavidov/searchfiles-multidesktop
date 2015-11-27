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
****************************************************************************/
#include "precompiled.h"
#if 1
#include "config.h"
#include <QObject>
#include <QSettings>
#endif

namespace Overskys
{
    const int Cfg::productLevel = 1;

    const int Cfg::shredder                 = 1;
    const int Cfg::fileMgr                  = 2;
    const bool Cfg::deepDel                 = true;

#ifdef OVERSKYS_SHREDDER
    const int Cfg::productType              = FilerConfig::shredder;
#else
    const int Cfg::productType              = Cfg::fileMgr;
#endif

    const QString Cfg::origDirPathKey       = QObject::tr("OrigDirPath");

//    const QString Cfg::deepDelKey           = QObject::tr("DeepDel");

//    const QString FilerConfig::deletePartialKey     = QObject::tr("DeletePartial");
//    const QString FilerConfig::copyPermissionsKey   = QObject::tr("CopyPermissions");
//    const QString FilerConfig::randomShredValuesKey = QObject::tr("RandomShredValues");
//    const QString FilerConfig::lastBrowsedLeftKey   = QObject::tr("LastBrowsedLeft");
//    const QString FilerConfig::lastBrowsedRightKey  = QObject::tr("LastBrowsedRight");
//    const QString FilerConfig::lastBrowsedLeftBKey  = QObject::tr("LastBrowsedLeftB");
//    const QString FilerConfig::lastBrowsedRightBKey = QObject::tr("LastBrowsedRightB");
//    const QString FilerConfig::useDockWidgetsKey    = QObject::tr("UseDockWidgets");
//    const QString FilerConfig::NameFilters          = QObject::tr("NameFilters");

//    const QString FilerConfig::AppGeometry          = QObject::tr("AppGeometry");
//    const QString FilerConfig::AppState             = QObject::tr("AppState");
//    const QString FilerConfig::View0Geometry        = QObject::tr("View0Geometry");
//    const QString FilerConfig::View1Geometry        = QObject::tr("View1Geometry");
//    const QString FilerConfig::View10Geometry       = QObject::tr("View10Geometry");
//    const QString FilerConfig::View11Geometry       = QObject::tr("View11Geometry");

//    const QString FilerConfig::column0Width         = QObject::tr("Column0Width");
//    const QString FilerConfig::column1Width         = QObject::tr("Column1Width");
//    const QString FilerConfig::column2Width         = QObject::tr("Column2Width");
//    const QString FilerConfig::column3Width         = QObject::tr("Column3Width");

    QSettings & Cfg::St()
    {
        static QSettings s_settings;
        return s_settings;
    }
}

