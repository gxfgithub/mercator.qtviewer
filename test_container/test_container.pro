#-------------------------------------------------
#
# Project created by QtCreator 2015-11-23T17:06:06
#
#-------------------------------------------------

QT       += core gui
win32: QT+= axcontainer
linux: QMAKE_CXXFLAGS += -std=c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_container
TEMPLATE = app
DESTDIR = $$OUT_PWD/../bin
LIBS += -L$$DESTDIR

LIBS += -lqtwidget_planetosm
INCLUDEPATH += $$PWD/../qtviewer_planetosm

SOURCES += main.cpp
HEADERS  += testcontainer.h
#if you want to build activec ctrls, please uncomment this line
DEFINES += BUILD_ACTIVEX_OSM

win32:contains(DEFINES,BUILD_ACTIVEX_OSM){
    FORMS    += testcontainer.ui
    SOURCES += testcontainer.cpp
win32-g++{
	QMAKE_CXXFLAGS += -std=c++11
}
} else {
    FORMS    += testcontainer_linux.ui
    SOURCES += testcontainer_linux.cpp
}

