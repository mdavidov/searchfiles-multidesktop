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
#include "getinfo.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    try
    {
        QApplication app(argc, argv);

        QCoreApplication::setOrganizationName(   OvSk_FsOp_COPMANY_NAME_TXT );
        QCoreApplication::setOrganizationDomain( OvSk_FsOp_COPMANY_DOMAIN_TXT );
        QCoreApplication::setApplicationName(    OvSk_FsOp_APP_NAME_TXT );
		QCoreApplication::setApplicationVersion( OvSk_FsOp_APP_VERSION_STR);
        Q_INIT_RESOURCE(getinfo);
        app.setWindowIcon(QIcon(":/images/getinfo.png"));

        Devonline::MainWindow mw(QDir::currentPath());
        mw.show();

        return app.exec();
    }
    catch (...)
    {
        Q_ASSERT(false);
        return 1;
    }
}
