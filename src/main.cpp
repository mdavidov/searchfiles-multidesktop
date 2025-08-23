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
#include "foldersearch.hpp"
#include "version.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    try {
        QApplication app(argc, argv);
        QCoreApplication::setOrganizationName(   OvSk_FsOp_COPMANY_NAME_TXT );
        QCoreApplication::setOrganizationDomain( OvSk_FsOp_COPMANY_DOMAIN_TXT );
        QCoreApplication::setApplicationName(   OvSk_FsOp_APP_NAME_TXT );
        QCoreApplication::setApplicationVersion( OvSk_FsOp_APP_VERSION_STR);
        Q_INIT_RESOURCE(foldersearch);
        app.setWindowIcon(QIcon(":/images/foldersearch.png"));

        QWidget qw;
        mmd::MainWindow wnd(QDir::currentPath(), &qw);
        wnd.show();

        return app.exec();
    }
    catch (const std::exception& ex) {
        qDebug() << "EXCEPTION: " << ex.what();
    }
    catch (...) {
        qDebug() << "caught ... EXCEPTION";
    }
}
