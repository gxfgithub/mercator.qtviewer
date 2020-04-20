#ifndef GEOGRAPHICSMULTILINEITEM_H
#define GEOGRAPHICSMULTILINEITEM_H

#include <QGraphicsPathItem>
#include <QPolygonF>
#include "geoitembase.h"
namespace QTVP_GEOMARKER{
	class geoGraphicsMultilineItem : public QGraphicsPathItem, public geoItemBase
	{
	protected:
		QPolygonF m_llap;
		void unwarrp();
	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
		void adjust_coords(int nNewLevel) override;
		QPointF label_pos() override;
	public:
		explicit geoGraphicsMultilineItem(QString name,QTVOSM::viewer_interface * pVi,
									 const QPolygonF & lla_polygon
				);
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
		QRectF boundingRect() const override;
	public:
		QPolygonF llas() const {return m_llap;}
		void setGeo(const QPolygonF & lla_polygon);
	};
}
#endif // geoGraphicsMultilineItem_H
