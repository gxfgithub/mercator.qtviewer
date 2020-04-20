﻿#ifndef QTVPLUGIN_GEOMARKER_H
#define QTVPLUGIN_GEOMARKER_H

#include <QWidget>
#include <QTranslator>
#include <QStandardItemModel>
#include <QMap>
#include <QXmlStreamAttributes>
#include <functional>
#include "geographicsscene.h"
#include "../qtviewer_planetosm/osmtiles/layer_interface.h"
#include "../qtviewer_planetosm/osmtiles/viewer_interface.h"
#include "geographicspixmapitem.h"
#include "geoitembase.h"
namespace Ui {
	class qtvplugin_geomarker;
}
using namespace QTVOSM;

/*!
 \brief qtvplugin_geomarker introduces QGraphicesView system, established a common approach for geo marking.
 GEO marker is a vector symbol that will be displayed on the background OSM raster map. there are 3 different mark types supported by this plugin.
 1. Point Marks. Include ellipse and rect style mark. these type of mark stand for a single point on earth, with a specified lat, lon .the width and height
 for rect / ellipse circumrect can be specified by user at runtime, in PIXEL. width and height will stay still during map zoom.

 2. Line Mark.	Line mark is a beeline on map. ATTENTION, in Mercator Projection system, the geo shortest path between 2 points on earth is NOT a beeline, that means
 beeline on a map is just for display performance and accessibility. the real path is a curve , which has 2 point of intersections exactly at start position and end position.

 3. Polygon (Polygon) Mark. Polygon mark is a polygon on map. borders of a polygon is painted with lines, for a same reason above, the geo shortest path between 2 points on earth is NOT a beeline either.

 Marks above shares a same style system provided by Qt painter system. pen, brush , font can be setted for each mark.

 Each mark has a Uinque ID called "name", and a type enum called "type". It can also contain several user-defined properties, with a key-value  style storage.
 Especial, a user-defined property called "LABEL"(Upper case) is different agains others. the value of LABEL will be displayed as text items on map all the time,
 but other properties will only visible when user click the mark.

 When the mark is clicked, or double clicked, a event will be fired, so that all plugins and OCX containers will be noticed that a mark (with ID) is clicked.

 \class qtvplugin_geomarker qtvplugin_geomarker.h "qtvplugin_geomarker/qtvplugin_geomarker.h"
*/
class qtvplugin_geomarker : public QWidget,public layer_interface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID OSMLayerInterface_iid )
	Q_INTERFACES(QTVOSM::layer_interface)
	typedef QMap<QString, QVariant> (tpfunc_call)(const QMap<QString, QVariant> &);
private:
	struct tag_xml_mark{
		QString name;
		int		type;
		QPolygonF geoPoints;
		QMap<QString, QString> styles;
		QMap<QString, QString> props;
	};
	struct tag_style_item{
		int		n_point_width;
		int		n_point_height;
		int		n_point_rect;
		QPen	pen;
		QBrush	brush;
		QColor	text_color;
		QFont	font;
		qreal	rotate;
		qreal	scale;
		int		multiline;
		int		smooth;
		QString icon_name;
	};

	//The global icon map
	QMap<QString,QTVP_GEOMARKER::tag_icon> m_map_icons;
	//Enum tools
	enum enum_tools_selection{
		TOOLS_DISPLAY_ONLY = 0,
		TOOLS_RECT_SELECTION = 1
	};
public:
	qtvplugin_geomarker(QWidget *parent = 0);
	~qtvplugin_geomarker();
	QWidget *	get_propWindow() {return this;}
	QString		get_name();
	/*! Get graphicalScence, the scence is a World coord in current level.
	 * in plugin system and draw tasks, user should divide class from  QGraphicsItem
	 * and maintain lat, lon coords to world coords according to current level
	 * (through  CV_LLA2World for convenience).
	*/
	QTVP_GEOMARKER::geoGraphicsScene * scene(){return m_pScene;}
private:
	int m_nInstance;
	QTVP_GEOMARKER::geoGraphicsScene * m_pScene;	//! the graphics scene object pointer.
	QTranslator pluginTranslator;
	Ui::qtvplugin_geomarker *ui;
	viewer_interface * m_pVi;						//! viewer_interface interface provides coordinats convertions.
	bool m_bVisible;
	QString m_SLLibName;
	QString m_SLLibPath;
	//selections
private:
	//current tools here
	enum_tools_selection m_currentTools;
	//selection tool
	QPointF m_sel_ptStart_World;
	QPointF m_sel_ptEnd_World;
	QSet<QString> m_set_itemNameSelected;
	QRectF CV_RectWrold2Mkt(QRectF world);
	QRectF current_sel_RectWorld();
	void addSelection(QRectF rectWorld);
	void clearSelection();
private:
	/*! a timer provides timing ui-refresh , instead of immediately refresh when item has been updated.
	 * This timer is just affects UI widgets, Map will be updated immediately otherwise.
	*/
	int m_nTimerID_refreshUI;
	int m_nTimerID_refreshMap;
	quint64 m_nDivideTimer;
	bool m_bNeedRefresh;
	bool m_bNeedUpdateView;

	QStandardItemModel * m_pLineStyleModel;
	QStandardItemModel * m_pFillStyleModel;
	QStandardItemModel * m_pGeoItemModel;
	QStandardItemModel * m_pGeoPropModel;
	QStandardItemModel * m_pIconsModel;
	QStandardItemModel * m_pSelItemNameModel;

	//persistent functions
private:
	//default style
	tag_style_item				m_default_style;
	QString		ini_file();
	void		ini_save();
	void		ini_load();
	void		style_save();
	void		style_load();
	bool		xml_save		(QString xml);
	bool		xml_load		(QString xml);
	bool		xml_readMark	(QXmlStreamReader & reader, tag_xml_mark & mark,QString & errMsg);
	bool		xml_readGeo		(QXmlStreamReader & reader, tag_xml_mark & mark,QString & errMsg);
	bool		xml_readStyle	(QXmlStreamReader & reader, tag_xml_mark & mark,QString & errMsg);
	bool		xml_readProps	(QXmlStreamReader & reader, tag_xml_mark & mark,QString & errMsg);
	QMap<QString,QString> xml_attribs_map(const QXmlStreamAttributes & ats);

	bool		xml_icon_save	(QString xml);
	bool		xml_icon_load	(QString xml);

	bool		cmd_save		(QString cmdFile);
	bool		cmd_load		(QString cmdFile);

	//UI refreshing functions
private:
	QList<QString> m_items_to_insert;
	void		scheduleRefreshMarks();
	void		scheduleUpdateMap();
	void		refresh_selection_listview();
	void		refreshItemUI(QString markname);
	void		refreshProps(QTVP_GEOMARKER::geoItemBase * itm);
	QColor		string2color(const QString & s);
	QString		color2string(const QColor & c);
	void		refreshIconModel();
	void		loadTranslations();
	bool		too_many_items();
	//Geo mark updating functions
private:
	//update methopd for UI
	template <class T>
	QTVP_GEOMARKER::geoItemBase *	update_point		(const QString & name,double lat, double lon, int width, int height, QPen pen, QBrush brush);
	QTVP_GEOMARKER::geoItemBase *	update_line			(const QString & name,double lat1, double lon1,double lat2, double lon2, QPen pen);
	QTVP_GEOMARKER::geoItemBase *	update_polygon		(const QString & name,const QPolygonF latlons, QPen pen, QBrush brush, bool tp);
	QTVP_GEOMARKER::geoItemBase *	update_icon			(const QString & name,double lat, double lon,qreal scale, qreal rotate,int smooth, QString id);
	//update method for XML
	bool							xml_update_mark		(tag_xml_mark & mark);
	//update method for plugin function calls
	QMap<QString, QVariant>			func_update_point	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_update_line	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_update_polygon	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_update_icon	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_update_props	(const QMap<QString, QVariant> &);
	//other method for plugin function calls
private:
	void initialBindPluginFuntions();
	QMap<QString,std::function <tpfunc_call> > m_map_pluginFunctions;
	QMap<QString, QVariant>			func_exists			(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_delete_marks	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_delete_props	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_mark_names		(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_mark			(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_props			(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_save_xml		(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_load_xml		(const QMap<QString, QVariant> &);
	//icon and its manage
	QMap<QString, QVariant>			func_add_resource	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_save_resources	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_load_resources	(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_props_vis		(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_show_props		(const QMap<QString, QVariant> &);
private:
	//tools methods
	QMap<QString, QVariant>			func_set_mod		(const QMap<QString, QVariant> &);
	//selection methods
	QMap<QString, QVariant>			func_selection_clear(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_selection_delete(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_selected_items	(const QMap<QString, QVariant> &);
	//style setting
	QMap<QString, QVariant>			func_set_default_style(const QMap<QString, QVariant> &);
	QMap<QString, QVariant>			func_default_style	(const QMap<QString, QVariant> &);

	//overloaded virtual funtions
protected:
	layer_interface *		load_initial_plugin(QString strSLibPath,viewer_interface  * ptrviewer);
	QWidget *				load_prop_window();
	void					load_retranslate_UI();

	bool		is_visible();
	void		set_visible(bool vb);
	bool		is_active(){return m_currentTools==qtvplugin_geomarker::TOOLS_DISPLAY_ONLY?false:true;}
	void		set_active(bool ab);
	void		set_name(QString vb);
	bool		is_exclusive(){return true;}

	void		cb_paintEvent( QPainter * pImage );
	void		cb_levelChanged(int);
	bool		cb_mouseMoveEvent(QMouseEvent *);
	bool		cb_mousePressEvent(QMouseEvent *);
	bool		cb_mouseReleaseEvent ( QMouseEvent * /*event*/ );
	bool		cb_mouseDoubleClickEvent(QMouseEvent *);
	bool		cb_event(const QMap<QString, QVariant>);

	void					timerEvent(QTimerEvent * e);
	QMap<QString, QVariant> call_func(const  QMap<QString, QVariant> & /*paras*/);
	//ui slots
protected slots:
	void on_pushButton_QTV_update_clicked();
	void on_radioButton_QTV_tool_point_toggled(bool);
	void on_radioButton_QTV_tool_line_toggled(bool);
	void on_radioButton_QTV_tool_polygon_toggled(bool);
	void on_radioButton_QTV_tool_icons_toggled(bool);
	void on_toolButton_QTV_selColorPen_clicked();
	void on_toolButton_QTV_selColorFill_clicked();
	void on_toolButton_QTV_selColorText_clicked();
	void on_tableView_QTV_marks_doubleClicked(const QModelIndex & index);
	void on_pushButton_QTV_prop_update_clicked();
	void on_pushButton_QTV_prop_delete_clicked();
	void on_pushButton_QTV_del_clicked();
	void on_pushButton_QTV_pickToLine1_clicked();
	void on_pushButton_QTV_pickToLine2_clicked();
	void on_pushButton_QTV_getPolygon_clicked();
	void on_pushButton_QTV_save_clicked();
	void on_pushButton_QTV_load_clicked();
	void on_pushButton_QTV_import_icon_clicked();
	void on_pushButton_QTV_load_icons_clicked();
	void on_pushButton_QTV_save_icons_clicked();
	void on_pushButton_QTV_refresh_list_clicked();
	void on_pushButton_QTV_collaps_all_clicked();
	void on_radioButton_QTV_display_clicked();
	void on_radioButton_QTV_rect_selection_clicked();
	void on_pushButton_QTV_sel_clear_clicked();
	void on_pushButton_QTV_sel_delselected_clicked();
	void on_pushButton_QTV_style_default_save_clicked();
	void on_pushButton_QTV_style_default_load_clicked();
};

template <class T>
QTVP_GEOMARKER::geoItemBase * qtvplugin_geomarker::update_point(const QString & name,double lat, double lon, int width, int height, QPen pen, QBrush brush)
{
	QTVP_GEOMARKER::geoItemBase * res = 0;
	//Get raw Item by name
	QTVP_GEOMARKER::geoItemBase * base = m_pScene->geoitem_by_name(name);
	//Get Props
	QStringList propNames;
	QVariantList propValues;
	if (base)
	{
		propNames = base->prop_names();
		propValues = base->prop_values();
	}
	//type convertion to T
	T * pitem = base?dynamic_cast<T  *>(base):0;
	if (!pitem)
		pitem	= new T(name,
						this->m_pVi,
						lat,lon,
						width,height);
	pitem->setPen(pen);
	pitem->setBrush(brush);

	if (base == pitem)
	{
		pitem->setGeo(lat,lon);
		pitem->setSize(width,height);
		res = pitem;
	}
	else if (false==this->m_pScene->addItem(pitem,0))
	{
		if (base != pitem)
			delete pitem;
	}
	else
	{
		int cs = propNames.size();
		for (int i=0;i<cs && base != pitem;++i)
		{
			pitem->set_prop_data(propNames.first(), propValues.first());
			propNames.pop_front();
			propValues.pop_front();
		}
		res = pitem;
	}

	return res;

}

#endif // QTVPLUGIN_GEOMARKER_H


