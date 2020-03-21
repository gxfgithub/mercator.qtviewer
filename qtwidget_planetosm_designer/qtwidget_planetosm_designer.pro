#-------------------------------------------------
#
# Project created by QtCreator 2016-03-03T09:12:05
#
#-------------------------------------------------

QT       += widgets network designer
linux:QMAKE_CXXFLAGS += -std=c++11
win32-g++:QMAKE_CXXFLAGS += -std=c++11
TARGET = qtwidget_planetosm_designer
DESTDIR = $$OUT_PWD/../bin
TEMPLATE = lib

LIBS += -L$$DESTDIR
LIBS += -lqtwidget_planetosm

DEFINES += QTWIDGET_PLANETOSM_DESIGNER_LIBRARY

SOURCES += osm_designer_plugin.cpp

HEADERS += osm_designer_plugin.h\
	qtwidget_planetosm_designer_global.h


