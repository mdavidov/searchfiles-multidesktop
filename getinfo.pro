TEMPLATE  = app
CONFIG += c++20
CONFIG += precompile_header
QT += widgets
PRECOMPILED_HEADER  = src/precompiled.h

HEADERS  =  \
    src/precompiled.h \
    src/common.h \
    src/config.h \
    src/version.h \
    src/util.h \
    src/getinfo.h \
    src/exlineedit.h \
    src/searchlineedit.h

SOURCES  =  \
    src/util.cpp \
    src/main.cpp \
    src/precompiled.cpp \
    src/config.cpp \
    src/getinfo.cpp \
    src/exlineedit.cpp \
    src/searchlineedit.cpp

# RESOURCES = src/dockwidgets.qrc

macx {
    INCLUDEPATH += /opt/qt/6.8.1/include
    LIBS += -L/opt/qt/6.8.1/lib
}

unix:!macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

win32 {
}

symbian {
}

INSTALLS += target

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)

RESOURCES += \
    src/dockwidgets.qrc

OTHER_FILES += \
    getinfo.todo.txt \
    src/aboutdialog.html \
    installers/inno/getinfo.bat \
    installers/inno/getinfo.iss

DISTFILES += \
    README.txt
