/****************************************************************************
**
** Copyright (c) 2010 Milivoj (Mike) Davidov
** All rights reserved.
**
** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
** EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "precompiled.h"
#include "config.h"
#include <QObject>
#include <QSettings>

namespace Devonline
{
    const int Cfg::productLevel = 1;

    const int Cfg::shredder                 = 1;
    const int Cfg::fileMgr                  = 2;
    const bool Cfg::deepDel                 = true;

#ifdef DEVONLINE_SHREDDER
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

