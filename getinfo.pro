TEMPLATE  = app
CONFIG += c++20
CONFIG += precompile_header
QT += widgets
PRECOMPILED_HEADER  = src/precompiled.h

HEADERS  =  \
    src/precompiled.h \
    src/aboutdialog.h \
    src/helpdialog.h \
    src/keypress.hpp \
    src/common.h \
    src/config.h \
    src/version.h \
    src/util.h \
    src/getinfo.h \
    src/exlineedit.h \
    src/searchlineedit.h

SOURCES  =  \
    src/precompiled.cpp \
    src/aboutdialog.cpp \
    src/helpdialog.cpp \
    src/keypress.cpp \
    src/util.cpp \
    src/main.cpp \
    src/config.cpp \
    src/getinfo.cpp \
    src/exlineedit.cpp \
    src/searchlineedit.cpp

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 42.0
    QMAKE_APPLE_DEVICE_ARCHS = arm64
    INCLUDEPATH += /opt/Qt/6.8.1/macos/lib/QtWidgets.framework/Versions/A/Headers
    INCLUDEPATH += /opt/Qt/6.8.1/macos/include
    LIBS += -L/opt/Qt/6.8.1/macos/lib
}

unix:!macx {
    INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6
    INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6/QtWidgets
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/lib/x86_64-linux-gnu
    LIBS += -L/usr/local/lib
}

win32 {
}

symbian {
}

INSTALLS += target

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)

RESOURCES += \
    getinfo.qrc

OTHER_FILES += \
    TODO.md \
    src/aboutdialog.html \
    installers/inno/getinfo.bat \
    installers/inno/getinfo.iss

DISTFILES += \
    README.md
