include (../../../../config.pri)
include (../../../../build.pri)

QT       += core dbus gui network concurrent xml

TARGET = simulator_fs9
TEMPLATE = lib

CONFIG += plugin shared
CONFIG += blackmisc blackcore

LIBS +=  -lsimulator_fscommon -lFSUIPC_User -luuid

# required for FSUIPC
win32:!win32-g++*: QMAKE_LFLAGS += /NODEFAULTLIB:LIBC.lib

DEPENDPATH += . ../../../../src
INCLUDEPATH += . ../../../../src

LIBS += -ldxguid -lole32

SOURCES += *.cpp
HEADERS += *.h

win32:!win32-g++*: PRE_TARGETDEPS += ../../../../lib/blackmisc.lib \
                                     ../../../../lib/blackcore.lib
else:              PRE_TARGETDEPS += ../../../../lib/libblackmisc.a \
                                     ../../../../lib/libblackcore.a

DESTDIR = ../../../../bin/plugins/simulator
include (../../../../libraries.pri)
