﻿#include "geographicslineitem.h"
#include "../qtviewer_planetosm/osmtiles/viewer_interface.h"
#include <assert.h>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "geographicsscene.h"
#include "qtvplugin_geomarker.h"
namespace QTVP_GEOMARKER{
	geoGraphicsLineItem::geoGraphicsLineItem(
			QString name,
			QTVOSM::viewer_interface * pVi,
			qreal lat1 ,
			qreal lon1 ,
			qreal lat2 ,
			qreal lon2)
		:QGraphicsLineItem(0)
		,geoItemBase(name,QTVP_GEOMARKER::ITEAMTYPE_LINE,pVi)
		,m_lat1(lat1)
		,m_lon1(lon1)
		,m_lat2(lat2)
		,m_lon2(lon2)
	{
		assert(vi()!=0);
		double px1,py1,px2,py2;
		unwarrp();
		vi()->CV_LLA2World(m_lat1,m_lon1,&px1,&py1);
		vi()->CV_LLA2World(m_lat2,m_lon2,&px2,&py2);
		setLine(px1,py1,px2,py2);
	}

	void geoGraphicsLineItem::unwarrp()
	{
		while (m_lon2 - m_lon1 < -180)
			m_lon2 += 360;
		while (m_lon2 - m_lon1 > 180)
			m_lon2 -= 360;

		while (m_lon1 < -180)
		{
			m_lon1 += 360;
			m_lon2 += 360;
		}
		while (m_lon1 >180)
		{
			m_lon1 -= 360;
			m_lon2 -= 360;
		}
	}

	void geoGraphicsLineItem::adjust_coords(int nNewLevel)
	{
		if (vi() && nNewLevel != level())
		{
			/** Since the map is zooming from level() to current level,
			 * the map size zoom ratio can be calculated using pow below.
			 * We can get new coord for current zoom level by multiplicative.
			*/
			double ratio = pow(2.0,(nNewLevel - level()));
			QLineF l1 = this->line();
			setLine(l1.x1() * ratio,l1.y1() * ratio,l1.x2() * ratio,l1.y2() * ratio);
		}
	}
	void geoGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= nullptr*/)
	{
		if (vi())
		{
			const int cv = vi()->level();
			if (cv!=level())
			{
				adjust_coords(cv);
				setLevel(cv);
			}
		}

		QGraphicsLineItem::paint(painter,option,widget);
	}
	QRectF geoGraphicsLineItem::boundingRect() const
	{
		QRectF rect = QGraphicsLineItem::boundingRect();
		auto v = vi();
		if (v && v->level()!=level())
		{
			double ratio = pow(2.0,(v->level() - level()));
			QPointF center = rect.center();
			rect.setRect(center.x() * ratio - rect.width()/2,
					center.y() * ratio - rect.height()/2,
					rect.width(),
					rect.height());
		}
		return rect;
	}
	void geoGraphicsLineItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
	{
		QGraphicsLineItem::mousePressEvent(event);
		//if (wantMouseHoverEvent()==false)
		{
			bool bshow = this->props_visible();
			this->show_props(!bshow);
		}
		//post enent
		QMap<QString, QVariant > map_evt;
		geoGraphicsScene * pscene = dynamic_cast<geoGraphicsScene *>(this->scene());
		if (pscene)
		{
			QObject * pPlg = pscene->parent();
			if (pPlg)
			{
				qtvplugin_geomarker * pMarker = dynamic_cast<qtvplugin_geomarker *>(pPlg) ;
				if (pMarker)
				{
					map_evt["source"] = pMarker->get_name();
					map_evt["destin"] = "ALL";
					if (event->buttons() & Qt::LeftButton)
						map_evt["name"] = "ITEM_LBUTTON_CLICKED";
					else if (event->buttons() & Qt::RightButton)
						map_evt["name"] = "ITEM_RBUTTON_CLICKED";
					else if (event->buttons() & Qt::MidButton)
						map_evt["name"] = "ITEM_MBUTTON_CLICKED";
					else
						map_evt["name"] = "ITEM_BUTTON_CLICKED";
					map_evt["id"] = this->item_name();
					vi()->post_event(map_evt);
				}
			}

		}
	}
	void geoGraphicsLineItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
	{
		QGraphicsLineItem::hoverEnterEvent(event);
		//this->show_props(true);
		//post enent
		QMap<QString, QVariant > map_evt;
		geoGraphicsScene * pscene = dynamic_cast<geoGraphicsScene *>(this->scene());
		if (pscene)
		{
			QObject * pPlg = pscene->parent();
			if (pPlg)
			{
				qtvplugin_geomarker * pMarker = dynamic_cast<qtvplugin_geomarker *>(pPlg) ;
				if (pMarker)
				{
					map_evt["source"] = pMarker->get_name();
					map_evt["destin"] = "ALL";
					map_evt["name"] = "ITEM_MOUSE_ENTER";
					map_evt["id"] = this->item_name();
					vi()->post_event(map_evt);
				}
			}

		}
	}

	void geoGraphicsLineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
	{
		QGraphicsLineItem::hoverLeaveEvent(event);
		//this->show_props(false);
		//post enent
		QMap<QString, QVariant > map_evt;
		geoGraphicsScene * pscene = dynamic_cast<geoGraphicsScene *>(this->scene());
		if (pscene)
		{
			QObject * pPlg = pscene->parent();
			if (pPlg)
			{
				qtvplugin_geomarker * pMarker = dynamic_cast<qtvplugin_geomarker *>(pPlg) ;
				if (pMarker)
				{
					map_evt["source"] = pMarker->get_name();
					map_evt["destin"] = "ALL";
					map_evt["name"] = "ITEM_MOUSE_LEAVE";
					map_evt["id"] = this->item_name();
					vi()->post_event(map_evt);
				}
			}

		}
	}
	void geoGraphicsLineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
	{
		QGraphicsLineItem::mouseDoubleClickEvent(event);
		//post enent
		QMap<QString, QVariant > map_evt;
		geoGraphicsScene * pscene = dynamic_cast<geoGraphicsScene *>(this->scene());
		if (pscene)
		{
			QObject * pPlg = pscene->parent();
			if (pPlg)
			{
				qtvplugin_geomarker * pMarker = dynamic_cast<qtvplugin_geomarker *>(pPlg) ;
				if (pMarker)
				{
					map_evt["source"] = pMarker->get_name();
					map_evt["destin"] = "ALL";
					if (event->buttons() & Qt::LeftButton)
						map_evt["name"] = "ITEM_LBUTTON_DBLCLICKED";
					else if (event->buttons() & Qt::RightButton)
						map_evt["name"] = "ITEM_RBUTTON_DBLCLICKED";
					else if (event->buttons() & Qt::MidButton)
						map_evt["name"] = "ITEM_MBUTTON_DBLCLICKED";
					else
						map_evt["name"] = "ITEM_BUTTON_DBLCLICKED";
					map_evt["id"] = this->item_name();
					vi()->post_event(map_evt);
				}
			}

		}
	}

	QPointF geoGraphicsLineItem::label_pos()
	{
		return QPointF(
					(line().x1() + line().x2())/2
					,
					(line().y1() + line().y2())/2 );
	}

	void geoGraphicsLineItem::setGeo(qreal lat1, qreal lon1, qreal lat2, qreal lon2)
	{
		m_lat1 = lat1;	m_lon1 = lon1;
		m_lat2 = lat2;	m_lon2 = lon2;
		unwarrp();
		double px1,py1,px2,py2;
		vi()->CV_LLA2World(m_lat1,m_lon1,&px1,&py1);
		vi()->CV_LLA2World(m_lat2,m_lon2,&px2,&py2);
		setLine(px1,py1,px2,py2);
		adjustLabelPos();
	}
}
