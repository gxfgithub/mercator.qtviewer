#include "osm_frame_widget.h"
#include "ui_osm_frame_widget.h"
#include <QDir>
#include <QtPlugin>
#include "osmtiles/layer_tiles.h"
#include "osmtiles/layer_browser.h"
#include <QModelIndexList>
#include <QModelIndex>
#include <QPluginLoader>
#include <QFileDialog>
#include <QSettings>
#include <QSet>
#include <QList>
#include <QMessageBox>
#include "osmtiles/viewer_interface.h"
#include "interface_utils.h"
 QMutex osm_frame_widget::m_mutex_proteced;

/*!
 \brief osm_frame_widget is the main widget of this control.
 in this constructor, 2 OUTER message will be fired.

 \fn osm_frame_widget::osm_frame_widget
 \param parent
*/
osm_frame_widget::osm_frame_widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::osm_frame_widget)
{
	m_mutex_proteced.lock();
	QTVOSM_DEBUG("The osm_frame_widget class constructing...");
	ui->setupUi(this);
	m_pLayerDispMod = new QStandardItemModel(this);
	m_pLayerDispMod->setColumnCount(3);
	m_pLayerDispMod->setHeaderData(0,Qt::Horizontal,QString(tr("name")));
	m_pLayerDispMod->setHeaderData(1,Qt::Horizontal,QString(tr("active")));
	m_pLayerDispMod->setHeaderData(2,Qt::Horizontal,QString(tr("visible")));
	ui->tableView_QTV_layers->setModel(m_pLayerDispMod);
	connect(ui->widget_QTV_mainMap,&tilesviewer::evt_level_changed,ui->dial_QTV_zoom,&QDial::setValue);
	connect(ui->dial_QTV_zoom,&QDial::valueChanged,ui->widget_QTV_mainMap,&tilesviewer::setLevel);


	QSettings settings(QCoreApplication::applicationFilePath()+".ini",QSettings::IniFormat);
	QSet<QString> set_tilesNames;
	QList<QString> list_tilesNames;
	for (int i=0;i<8;++i)
	{
		QString lk = QString("tiles/name%1").arg(i);
		QString nk = settings.value(lk).toString().trimmed();
		if (nk.size() && set_tilesNames.contains(nk)==false)
		{
			set_tilesNames.insert(nk);
			list_tilesNames.push_back(nk);
		}
	}
	if (set_tilesNames.empty())
	{
		set_tilesNames.insert("OSM");
		list_tilesNames.push_back("OSM");
	}
	layer_tiles * pLastTileLayer = nullptr;
	int tc = 0;
	foreach(QString layerNameT, list_tilesNames)
	{
		//add tile layers
		layer_tiles * pTile =  new layer_tiles(ui->widget_QTV_mainMap);
		pTile->set_name(layerNameT);
		pTile->set_active(true);
		pTile->set_visible(true);
		AppendLayer(QCoreApplication::applicationFilePath(),pTile);
		pLastTileLayer = pTile;
		QString lk = QString("tiles/name%1").arg(tc++);
		settings.setValue(lk,layerNameT);
	}

	//add single layer to browser
	layer_browser * pOSMTileBr = new layer_browser(ui->browserView);
	pOSMTileBr->set_name(pLastTileLayer->get_name());
	pOSMTileBr->load_initial_plugin(QCoreApplication::applicationFilePath(),ui->browserView);
	ui->browserView->addLayer(pOSMTileBr);


	//connect center change event
	connect (ui->widget_QTV_mainMap,&tilesviewer::evt_center_changed,ui->browserView,&tilesviewer::setBrCenterLLA);
	connect (ui->browserView,&tilesviewer::evt_center_changed,ui->widget_QTV_mainMap,&tilesviewer::setCenterLLA);
	connect (ui->widget_QTV_mainMap,&tilesviewer::evt_level_changed,ui->browserView,&tilesviewer::setBrLevel);
	connect (ui->widget_QTV_mainMap,&tilesviewer::cmd_update_layer_box,this,&osm_frame_widget::delacmd_refresh_layer_view,Qt::QueuedConnection);
	//send messages
	//! 1. source=MAIN_MAP,  destin = ALL, msg = WINDOW_CREATE
	if (this->isEnabled())
	{
		QMap<QString, QVariant> map_evt;
		map_evt["source"] = "MAIN_MAP";
		map_evt["destin"] = "ALL";
		map_evt["name"] = "WINDOW_CREATE";
		ui->widget_QTV_mainMap->post_event(map_evt);
	}

	ui->tab_map->installEventFilter(this);
	//adjust layers, make exclusive layrs being de-activated.
	ui->widget_QTV_mainMap->adjust_layers(pLastTileLayer);
	EnumPlugins();
	UpdateLayerTable();
	//Dock is closable
	ui->dockWidget_QTV_side->installEventFilter(this);
	m_mutex_proteced.unlock();
	//! 2. source=MAIN_MAP,  destin = ALL, msg = MAP_INITED
	if ( this->isEnabled())
	{
		QMap<QString, QVariant> map_evt;
		map_evt["source"] = "MAIN_MAP";
		map_evt["destin"] = "ALL";
		map_evt["name"] = "MAP_INITED";
		map_evt["nLevel"] = ui->widget_QTV_mainMap->level();
		ui->widget_QTV_mainMap->post_event(map_evt);
	}
	QTVOSM_DEBUG("The osm_frame_widget class constructed.");

}
void osm_frame_widget::UpdateLayerTable()
{
	QVector<QString> names = ui->widget_QTV_mainMap->layerNames();
	QVector<bool> activities = ui->widget_QTV_mainMap->layerActivities();
	QVector<bool> visibles = ui->widget_QTV_mainMap->layerVisibilities();
	int nItems = names.size();
	if (m_pLayerDispMod->rowCount()>0)
		m_pLayerDispMod->removeRows(0,m_pLayerDispMod->rowCount());
	for (int i=0;i<nItems;++i)
	{
		m_pLayerDispMod->appendRow(new QStandardItem(names[nItems-1-i]));
		m_pLayerDispMod->setData(m_pLayerDispMod->index(i,1),activities[nItems-1-i]);
		m_pLayerDispMod->setData(m_pLayerDispMod->index(i,2),visibles[nItems-1-i]);
	}
}
tilesviewer * osm_frame_widget::viewer()
{
	return ui->widget_QTV_mainMap;
}

bool osm_frame_widget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Close)
	{
		if (obj == ui->tab_map)
		{
			event->ignore();
			QWidget * wig = qobject_cast<QWidget *>(obj);
			if (wig)
			{
				Qt::WindowFlags flg = wig->windowFlags();
				flg &= ~(Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint|Qt::Window );
				wig->setWindowFlags(flg);
				int idx = ui->tabWidget_main->addTab(
							wig,
							"Map"
							);
				ui->tabWidget_main->setTabIcon(idx,wig->windowIcon());
				return true;
			}
		}
		else if (obj == ui->dockWidget_QTV_side)
		{
			event->ignore();
			enableLiteMode(true);
			return true;
		}
		else if (m_PropPageslayer.contains(obj))
		{
			event->ignore();
			QWidget * wig = qobject_cast<QWidget *>(obj);
			if (wig)
			{
				Qt::WindowFlags flg = wig->windowFlags();
				flg &= ~(Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint|Qt::Window );
				wig->setWindowFlags(flg);
				int idx = ui->tabWidget_main->addTab(
							wig,
							m_PropPageslayer[obj]->get_name()
							);
				ui->tabWidget_main->setTabIcon(idx,wig->windowIcon());
			}
			return true;
		}
		else
		{

		}

	}
	// standard event processing
	return QObject::eventFilter(obj, event);
}

bool osm_frame_widget::AppendLayer(QString SLName,layer_interface * interface)
{
	layer_interface * ci = interface->load_initial_plugin(SLName,ui->widget_QTV_mainMap);
	if (0==ci)
		return false;
	if (false==ui->widget_QTV_mainMap->addLayer(ci))
		return false;
	QWidget * wig = ci->load_prop_window();
	if (wig)
	{
		m_layerPropPages[ci] = wig;
		m_PropPageslayer[wig] = ci;
		int idx = ui->tabWidget_main->addTab(wig,ci->get_name());
		ui->tabWidget_main->setTabIcon(idx,wig->windowIcon());
		wig->installEventFilter(this);
	}
	return true;
}

osm_frame_widget::~osm_frame_widget()
{
	delete ui;
}
void osm_frame_widget::mousePressEvent(QMouseEvent * e)
{
	if (e->pos().x() >= this->rect().right()-12 && m_bLiteModeLocked==false)
	{
		if (ui->dockWidget_QTV_side->isVisible()==false)
		{
			enableLiteMode(false);
		}
	}

	QWidget::mousePressEvent(e);
}
void osm_frame_widget::EnumPlugins()
{
	QTVOSM_DEBUG("The osm_frame_widget is enuming plugins.");
	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir pluginsDir(strAppDir);
	QStringList filters;
	filters << "qtvplugin_*.dll" << "libqtvplugin_*.so";
	pluginsDir.setNameFilters(filters);
	//Enum files
	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = pluginLoader.instance();//try to load Plugins
		if (plugin) {
			layer_interface * pPlugin= qobject_cast<layer_interface *>(plugin);
			if (pPlugin)
			{
				if (false==AppendLayer(fileName,pPlugin))
				{

				}
			}
		}
	}
	QTVOSM_DEBUG("The osm_frame_widget loaded all plugins.");
	return ;
}

void osm_frame_widget::on_pushButton_QTV_visible_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	int nItems = layers.size();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			layers[nItems - 1 -row]->set_visible(true);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();

		}
	}
}

void osm_frame_widget::on_pushButton_QTV_hide_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			layers[nItems - 1 -row]->set_visible(false);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();
		}
	}
}

void osm_frame_widget::on_pushButton_QTV_moveDown_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			ui->widget_QTV_mainMap->moveLayerUp(layers[nItems - 1 -row]);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();
		}
	}
}

void osm_frame_widget::on_pushButton_QTV_moveBtm_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			ui->widget_QTV_mainMap->moveLayerTop(layers[nItems - 1 -row]);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();
		}
	}
}

void osm_frame_widget::on_pushButton_QTV_moveUp_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			ui->widget_QTV_mainMap->moveLayerDown(layers[nItems - 1 -row]);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();
		}
	}
}

void osm_frame_widget::on_pushButton_QTV_moveTop_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			ui->widget_QTV_mainMap->moveLayerBottom(layers[nItems - 1 -row]);
			UpdateLayerTable();
			ui->widget_QTV_mainMap->UpdateWindow();
		}
	}
}
void osm_frame_widget::delacmd_refresh_layer_view()
{
	UpdateLayerTable();
	ui->widget_QTV_mainMap->UpdateWindow();
}

void osm_frame_widget::on_pushButton_QTV_active_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			for (int i=0;i<layers.size();++i)
			{
				//It's exclusive, there should be at most only one layer_tiles active
				if (i==(nItems - 1 -row))
				{
					layers[i]->set_active(true);
					ui->widget_QTV_mainMap->adjust_layers(layers[i]);
				}
			}
			UpdateLayerTable();

		}
	}
}

void osm_frame_widget::on_pushButton_QTV_deactive_clicked()
{
	QVector <layer_interface *> layers = ui->widget_QTV_mainMap->layers();
	QModelIndexList lstSel = ui->tableView_QTV_layers->selectionModel()->selectedIndexes();
	int nItems = layers.size();
	if (lstSel.size())
	{
		int row = lstSel.first().row();
		if (row >=0 && row < layers.size())
		{
			for (int i=0;i<layers.size();++i)
			{
				if (i==(nItems - 1 -row))
					layers[i]->set_active(false);
			}
			UpdateLayerTable();

		}
	}
}

void osm_frame_widget::on_tabWidget_main_tabCloseRequested(int index)
{
	QWidget * wig = ui->tabWidget_main->widget(index);
	this->ui->tabWidget_main->removeTab(index);

	Qt::WindowFlags flg = wig->windowFlags();
	flg |= (Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint|Qt::Window );
	wig->setWindowFlags(flg);
	wig->show();
	wig->move(100,100);

}

void osm_frame_widget::on_pushButton_QTV_saveToFile_clicked()
{
	QSettings settings(QCoreApplication::applicationFilePath()+".ini",QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("history/last_save_img_dir","./").toString();

	QString strDefaultFilename = "Image_" +
			QDateTime::currentDateTime()
			.toString("yyyyMMddHHmmss") +".png";

	QFileDialog dlg_save(this,tr("save to image"),strLastSaveImgDir,
								 "Images (*.png *.bmp *.jpg);;All files(*.*)"
								 );
	dlg_save.setFileMode(QFileDialog::AnyFile);
	dlg_save.setAcceptMode(QFileDialog::AcceptSave);
	dlg_save.selectFile(strDefaultFilename);
	if (dlg_save.exec()==QDialog::Accepted)
	{
		QStringList fms = dlg_save.selectedFiles();
		foreach (QString newfm,  fms)
		{
			if (true == ui->widget_QTV_mainMap->saveToImage(newfm))
			{
				QFileInfo info(newfm);
				settings.setValue("history/last_save_img_dir",info.absolutePath());
			}
		}
	}
}
void osm_frame_widget::enableLiteMode(bool bEnabled)
{
	if (bEnabled==true)
	{
		if (ui->tab_map->parent()==this)
			return;
		ui->tabWidget_main->hide();
		int idx = ui->tabWidget_main->indexOf(ui->tab_map);
		if (idx>=0)
			ui->tabWidget_main->removeTab(idx);
		ui->tab_map->setParent(this);
		//remove topmost flag
		Qt::WindowFlags flg = ui->tab_map->windowFlags();
		flg &= ~(Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint|Qt::Window );
		ui->tab_map->setWindowFlags(flg);
		//add to current Layer
		this->layout()->addWidget(ui->tab_map);
		ui->tab_map->show();
		ui->dockWidget_QTV_side->hide();
		QMargins m = this->layout()->contentsMargins();
		this->layout()->setContentsMargins(m.left(),m.top(),12,m.bottom());

	}
	else
	{
		Qt::WindowFlags flg = ui->tab_map->windowFlags();
		flg &= ~(Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint|Qt::Window );
		ui->tab_map->setWindowFlags(flg);
		int idx = ui->tabWidget_main->addTab(
					ui->tab_map,
					"Map"
					);
		ui->tabWidget_main->setTabIcon(idx,ui->tab_map->windowIcon());
		ui->tabWidget_main->show();
		ui->tabWidget_main->setCurrentIndex(idx);
		QMargins m = this->layout()->contentsMargins();
		this->layout()->setContentsMargins(m.left(),m.top(),m.left(),m.bottom());
		ui->dockWidget_QTV_side->show();

	}
}
void osm_frame_widget::lockLiteMode(bool blocked)
{
	m_bLiteModeLocked = blocked;
}
