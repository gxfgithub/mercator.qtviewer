#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T17:46:14
#
#-------------------------------------------------

QT       += core gui network designer
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
linux:QMAKE_CXXFLAGS += -std=c++11 -fPIC
win32-g++:QMAKE_CXXFLAGS += -std=c++11
TARGET = qtwidget_planetosm
DESTDIR = $$OUT_PWD/../bin
TEMPLATE    = lib
CONFIG += dll

DEFINES += PLANETOSM_EXPORT_DLL

SOURCES += \
    qtwidget_planetosm.cpp\
    osm_frame_widget.cpp \
    osmtiles/cProjectionMercator.cpp \
    osmtiles/tilesviewer.cpp \
    osmtiles/urlDownloader.cpp \
    osmtiles/layer_tiles.cpp \
    osmtiles/layer_browser.cpp \
    osmtiles/layer_tiles_page.cpp


HEADERS  += osm_frame_widget.h \
    osmtiles/cProjectionMercator.h \
    osmtiles/tilesviewer.h \
    osmtiles/urlDownloader.h \
    osmtiles/layer_tiles.h \
    osmtiles/layer_browser.h \
    osmtiles/layer_interface.h \
    osmtiles/viewer_interface.h \
    osmtiles/layer_tiles_page.h \
    qtwidget_planetosm.h

FORMS    += osm_frame_widget.ui \
    osmtiles/layer_tiles_page.ui

win32{
OTHER_FILES += \
	qtviewer_planetosm_zh_CN.ts
}

RESOURCES += \
    resource/resource.qrc

TRANSLATIONS += qtviewer_planetosm_zh_CN.ts

