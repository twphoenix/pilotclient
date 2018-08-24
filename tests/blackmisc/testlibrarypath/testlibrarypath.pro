load(common_pre)

QT += core dbus testlib

TARGET = testlibrarypath
CONFIG   -= app_bundle
CONFIG   += blackconfig
CONFIG   += blackmisc
CONFIG   += testcase
CONFIG   += no_testcase_installs

TEMPLATE = app

DEPENDPATH += \
    . \
    $$SourceRoot/src \
    $$SourceRoot/tests \

INCLUDEPATH += \
    $$SourceRoot/src \
    $$SourceRoot/tests \

SOURCES += testlibrarypath.cpp

DESTDIR = $$DestRoot/bin

load(common_post)