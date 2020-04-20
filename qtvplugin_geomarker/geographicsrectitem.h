#ifndef GEOGRAPHICSRECTITEM_H
#define GEOGRAPHICSRECTITEM_H

#include <QGraphicsRectItem>
#include "geoitembase.h"
namespace QTVP_GEOMARKER{
	class geoGraphicsRectItem : public QGraphicsRectItem, public geoItemBase
	{
	protected:
		qreal m_lat;
		qreal m_lon;
		qreal m_width;
		qreal m_height;
	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
		void adjust_coords(int ncurrLevel) override;
	public:
		explicit geoGraphicsRectItem(QString name,QTVOSM::viewer_interface * pVi,
							   qreal cent_lat = 90,
							   qreal cent_lon = 0,
							   qreal pxwidth = 8,
							   qreal pxheight = 8);
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
		QRectF boundingRect() const override;
	public:
		qreal lat() const {return m_lat;}
		qreal lon() const {return m_lon;}
		qreal width() const {return m_width;}
		qreal height() const {return m_height;}
		void setSize(qreal pxwidth,qreal pxheight);
		void setGeo(qreal cent_lat,qreal cent_lon);
		QPointF label_pos() override;
	};
}
#endif // GEOGRAPHICSELLIPSEITEM_H
