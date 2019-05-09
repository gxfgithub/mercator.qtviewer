TEMPLATE = subdirs
#if you want to build activec ctrls, please uncomment this line
DEFINES += BUILD_ACTIVEX_OSM
SUBDIRS += \
	qtwidget_planetosm \
	qtwidget_planetosm_designer \
	qtviewer_planetosm \
	qtvplugin_grid \
	qtvplugin_geomarker \
	test_container

qtwidget_planetosm.file = qtviewer_planetosm/qtwidget_planetosm.pro

win32:{
contains(DEFINES,BUILD_ACTIVEX_OSM){
    SUBDIRS +=\
	qtaxviewer_planetosm
    qtaxviewer_planetosm.file = qtviewer_planetosm/qtaxviewer_planetosm.pro
}
}
