#include "geographicsscene.h"
#include "geoitembase.h"
#include <QGraphicsItem>
#include <algorithm>
namespace QTVP_GEOMARKER{
	geoGraphicsScene::geoGraphicsScene(QObject *parent) :
		QGraphicsScene(parent)
	{
		this->setItemIndexMethod(QGraphicsScene::NoIndex);
		currentNewLevel = 0;
	}
	geoGraphicsScene::geoGraphicsScene(const QRectF &sceneRect, QObject *parent )
		:QGraphicsScene(sceneRect,parent)
	{

	}

	geoGraphicsScene::geoGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent )
		:QGraphicsScene(x,y,width,height,parent)
	{

	}
	bool geoGraphicsScene::addItem(geoItemBase * item,int /*reserved*/)
	{
		bool res = false;
		QGraphicsItem * pg = dynamic_cast<QGraphicsItem *>(item);
		if (pg)
		{
			const QString & namec = item->item_name();
			if (m_map_items.contains(namec))
			{
				geoItemBase * oldItem = m_map_items[namec];
				if (oldItem != item)
				{
					this->removeItem(oldItem,0);
					this->m_map_items[namec] = item;
					this->addItem(pg);
				}
			}
			else
			{
				this->addItem(pg);
				this->m_map_items[namec] = item;
			}
			res = true;
		}

		return res;
	}

	void geoGraphicsScene::removeItem(geoItemBase * item, int /*reserved*/)
	{
		QGraphicsItem * pg = dynamic_cast<QGraphicsItem *>(item);
		if (pg)
		{
			QString c_name = item->item_name();
			if (m_map_items.contains(c_name))
			{
				this->removeItem(pg);
				m_map_items.remove(c_name);
				delete item;
			}
		}

	}


	geoItemBase * geoGraphicsScene::geoitem_by_name(const QString & name)
	{
		geoItemBase * it = 0;
		if (m_map_items.contains(name))
			it = m_map_items[name];
		return it;

	}

	QList<geoItemBase *> geoGraphicsScene::geo_items()
	{
		return /*std::move*/(m_map_items.values());
	}
	QList<QString> geoGraphicsScene::geo_item_names()
	{
		return /*std::move*/(m_map_items.keys());
	}
}
