#include "qtvplugin_geomarker.h"
#include "ui_qtvplugin_geomarker.h"
#include <QPainter>
#include <QBrush>
#include <QCoreApplication>
#include <QPen>
#include <QDebug>
#include <QLibraryInfo>
#include <QFileInfo>
#include <QMutex>
#include <QGraphicsSceneMouseEvent>
#include <QMap>
#include <QTextStream>
#include <assert.h>
#include "geographicsellipseitem.h"
#include "geographicsrectitem.h"
#include "geographicslineitem.h"
#include "geographicspolygonitem.h"
#include "geographicspixmapitem.h"
#include "geographicsmultilineitem.h"
#include "../qtviewer_planetosm/interface_utils.h"
QMutex mutex_instances;
QMap<viewer_interface *,  qtvplugin_geomarker * > map_instances;
QMap<QString,  int > count_instances;
qtvplugin_geomarker::qtvplugin_geomarker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::qtvplugin_geomarker),
    m_currentTools(qtvplugin_geomarker::TOOLS_DISPLAY_ONLY),
    m_sel_ptStart_World(0,0),
    m_sel_ptEnd_World(0,0)
{
    m_nInstance = 0;
    ui->setupUi(this);
    m_pVi = 0;
    m_nDivideTimer = 0;
    m_bVisible = true;
    m_pScene = new QTVP_GEOMARKER::geoGraphicsScene(this);
    m_pScene->setBackgroundBrush(Qt::NoBrush);
    //senece is 256 * 2^level
    m_pScene->setSceneRect(0,0,256,256);

    m_pGeoItemModel = new QStandardItemModel(this);
    m_pGeoItemModel->setColumnCount(3);
    m_pGeoItemModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    m_pGeoItemModel->setHeaderData(1,Qt::Horizontal,tr("Type"));
    m_pGeoItemModel->setHeaderData(2,Qt::Horizontal,tr("Props"));
    ui->tableView_QTV_marks->setModel(m_pGeoItemModel);

    m_pSelItemNameModel = new QStandardItemModel(this);
    m_pSelItemNameModel->setColumnCount(1);
    m_pSelItemNameModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    ui->tableView_QTV_marks_sel->setModel(m_pSelItemNameModel);


    m_pGeoPropModel = new QStandardItemModel(this);
    m_pGeoPropModel->setColumnCount(2);
    m_pGeoPropModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    m_pGeoPropModel->setHeaderData(1,Qt::Horizontal,tr("Value"));
    ui->tableView_QTV_props->setModel(m_pGeoPropModel);

    m_pLineStyleModel = new QStandardItemModel(this);
    m_pLineStyleModel->appendRow(new QStandardItem("NoPen"));
    m_pLineStyleModel->appendRow(new QStandardItem("SolidLine"));
    m_pLineStyleModel->appendRow(new QStandardItem("DashLine"));
    m_pLineStyleModel->appendRow(new QStandardItem("DotLine"));
    m_pLineStyleModel->appendRow(new QStandardItem("DashDotLine"));
    m_pLineStyleModel->appendRow(new QStandardItem("DashDotDotLine"));
    m_pLineStyleModel->appendRow(new QStandardItem("CustomDashLine"));
    ui->comboBox_QTV_linePad->setModel(m_pLineStyleModel);

    m_pFillStyleModel = new QStandardItemModel(this);
    m_pFillStyleModel->appendRow(new QStandardItem("NoBrush"));
    m_pFillStyleModel->appendRow(new QStandardItem("SolidPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense1Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense2Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense3Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense4Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense5Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense6Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("Dense7Pattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("HorPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("VerPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("CrossPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("BDiagPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("FDiagPattern"));
    m_pFillStyleModel->appendRow(new QStandardItem("DiagCrossPattern"));
    ui->comboBox_QTV_fillPad->setModel(m_pFillStyleModel);

    //insert 1 icons
    QTVP_GEOMARKER::tag_icon icon;
    icon.name = "default";
    icon.filename = "://icons/default.png";
    icon.centerx = 8;
    icon.centery = 8;
    if (icon.icon.load(icon.filename))
	m_map_icons[icon.name] = icon;


    m_pIconsModel = new QStandardItemModel(this);
    refreshIconModel();
    ui->comboBox_QTV_icons->setModel(m_pIconsModel);

    m_bNeedRefresh = false;
    m_bNeedUpdateView = false;
    m_nTimerID_refreshUI = startTimer(2000);
    m_nTimerID_refreshMap = startTimer(100);
    ui->radioButton_QTV_display->setChecked(true);
}

qtvplugin_geomarker::~qtvplugin_geomarker()
{
    delete ui;
}
void qtvplugin_geomarker::load_retranslate_UI()
{
    ui->retranslateUi(this);
}
/*! load_initial_plugin will be called when this plug in being loaded.
 * Please notice that a global parament "map_instances" is introduced, for multi-instance situation
 * in windows OCX usage. A Brave new instance of "qtvplugin_geomarker" will be created for each different viewer_interface.
 *
 * @param strSLibPath the absolute path of this dll.
 * @param ptrviewer the pointer to main view.
 * @return	return the instance pointer to the instance belong to ptrviewer
*/
layer_interface * qtvplugin_geomarker::load_initial_plugin(QString strSLibPath,viewer_interface  * ptrviewer)
{
    //!In this instance, we will see how to create a new instance for each ptrviewer
    qtvplugin_geomarker * instance = 0;
    //!1.Check whether there is already a instance for ptrviewer( viewer_interface)
    mutex_instances.lock();
    //!1.1 situation 1: map_instances is empty, which means no instance exists. We just save this pointer to map_instances
    if (map_instances.empty()==true)
    {
	map_instances[ptrviewer] = this;
	instance = this;
    }
    /*! 1.2 situation 2: map_instances dose not contain ptrviewer, which is the normal situation when a second ocx ctrl is initializing.
	 * we just allocate a new  qtvplugin_geomarker, and save key-value in map_instances.
	*/
    else if (map_instances.contains(ptrviewer)==false)
    {
	instance = new qtvplugin_geomarker;
	map_instances[ptrviewer] = instance;
    }
    //! 1.3 situation 3: a ABNORMAL situation. load_initial_plugin is called again.
    else
	instance = map_instances[ptrviewer];
    mutex_instances.unlock();
    //2. if the instance is just this object, we do real init code.
    if (instance==this)
    {
	QFileInfo info(strSLibPath);
	m_SLLibPath = strSLibPath;
	m_SLLibName = info.completeBaseName();
	m_pVi = ptrviewer;

	mutex_instances.lock();
	m_nInstance = ++count_instances[m_SLLibName];
	mutex_instances.unlock();

	loadTranslations();
	ini_load();
	style_load();
	initialBindPluginFuntions();
    }
    //3. elseif, we call the instance's load_initial_plugin method instead
    else
    {
	layer_interface * ret = instance->load_initial_plugin(strSLibPath,ptrviewer);
	assert(ret==instance);
	return ret;
    }
    qDebug()<<QFont::substitutions();
    return instance;
}

void qtvplugin_geomarker::loadTranslations()
{
    //Trans
    QCoreApplication * app =  QCoreApplication::instance();
    if (app && m_nInstance==1)
    {
	QString strTransLocalFile =
		QCoreApplication::applicationDirPath()+"/" +
		m_SLLibName+ "_" +
		QLocale::system().name()+".qm";
	if (true==pluginTranslator.load(strTransLocalFile ))
	{
	    QTVOSM_DEBUG("Load translationfile")<<"\n\t"<<strTransLocalFile<<" Succeeded.";
	    app->installTranslator(&pluginTranslator);
	    ui->retranslateUi(this);
	}
	else
	    QTVOSM_WARNING("Load translationfile")<<"\n\t"<<strTransLocalFile<<" Not Found.";

    }
}

QWidget * qtvplugin_geomarker::load_prop_window()
{
    return this;
}
bool qtvplugin_geomarker::too_many_items()
{
    bool res = false;
    if (!m_pVi || m_bVisible==false)
	return res;
    int currentLevel = m_pVi->level();
    if (currentLevel <=7)
    {
	//skip painting when there are too many marks on map
	int reduce_limit = (1<<currentLevel) * 4096;
	if (this->m_pScene->total_items()>=reduce_limit)
	    res = true;
    }
    return res;
}

void qtvplugin_geomarker::cb_paintEvent( QPainter * pImage )
{
    if (!m_pVi || m_bVisible==false)
	return ;
    QRect rect = m_pVi->windowRect();
    // Calc current viewport in world
    double leftcenx, topceny, rightcenx, bottomceny;
    m_pVi->CV_DP2World(0,0,&leftcenx,&topceny);
    m_pVi->CV_DP2World(rect.width()-1,rect.height()-1,&rightcenx,&bottomceny);

    int winsz = 256 * (1<<m_pVi->level());

    QRectF destin(
		0,
		0,
		rect.width(),
		rect.height()
		);
    //Warpping 180, -180. because longitude +180 and -180 is the same point,
    // but the map is plat, -180 and + 180 is quite different positions, we
    // should draw 3 times, to slove cross 180 drawing problems.
    for (int p = -1; p<=1 ;++p)
    {
	QRectF source(
		    leftcenx + p * winsz,
		    topceny,
		    (rightcenx - leftcenx),
		    (bottomceny - topceny)
		    );

	m_pScene->render(pImage,destin,source);

    }


    //draw current tools
    switch (m_currentTools)
    {
    case qtvplugin_geomarker::TOOLS_RECT_SELECTION:
    {
	QPen pen_sel(QColor(128,64,0,128));
	pen_sel.setWidth(3);
	pen_sel.setStyle(Qt::DashLine);
	pImage->setPen(pen_sel);
	pImage->drawText(32,32,"GEO MARKER Rect-Selection Tools Actived.");
	QRectF wrct = current_sel_RectWorld();
	if (wrct.isValid())
	{
	    double x1 = wrct.left();
	    double y1 = wrct.top();
	    double x2 = wrct.right();
	    double y2 = wrct.bottom();
	    qint32 nx1,ny1,nx2,ny2;
	    m_pVi->CV_World2DP(x1,y1,&nx1,&ny1);
	    m_pVi->CV_World2DP(x2,y2,&nx2,&ny2);
	    for (int i = -1;i<=1;++i)
	    {
		pImage->drawLine(nx1 + i * winsz,ny1,nx1 + i * winsz,ny2);
		pImage->drawLine(nx1 + i * winsz,ny2,nx2 + i * winsz,ny2);
		pImage->drawLine(nx2 + i * winsz,ny2,nx2 + i * winsz,ny1);
		pImage->drawLine(nx2 + i * winsz,ny1,nx1 + i * winsz,ny1);
	    }
	}
    }
	break;
    default:
	break;
    }
}

void qtvplugin_geomarker::cb_levelChanged(int level)
{
    if (!m_pVi)
	return ;
    m_sel_ptStart_World = m_sel_ptEnd_World = QPointF();
    //Adjust new Scene rect
    QRectF rect(0,0,256*(1<<level),256*(1<<level));
    this->set_visible(false);
    m_pScene->setSceneRect(rect);
    this->set_visible(true);
}

bool qtvplugin_geomarker::is_visible()
{
    return m_bVisible;
}

void qtvplugin_geomarker::set_visible(bool vb)
{
    m_bVisible = vb;
}
void qtvplugin_geomarker::set_active(bool ab)
{
    if (ab==true)
    {
	if (m_currentTools==qtvplugin_geomarker::TOOLS_DISPLAY_ONLY)
	{
	    ui->radioButton_QTV_rect_selection->setChecked(true);
	    m_currentTools = qtvplugin_geomarker::TOOLS_RECT_SELECTION;
	}
    }
    else
    {
	m_currentTools = qtvplugin_geomarker::TOOLS_DISPLAY_ONLY;
	ui->radioButton_QTV_display->setChecked(true);
    }

}

QString qtvplugin_geomarker::get_name()
{
    QString strName = m_SLLibName.mid(10);
    if (m_SLLibName.left(3)=="lib")
	strName = m_SLLibName.mid(13);
    if (strName.length())
	return strName/* + QString("%1").arg(m_nInstance)*/;
    else
	return "geomarker";
}

void qtvplugin_geomarker::set_name(QString /*vb*/)
{
    if (!m_pVi)
	return ;

}
QRectF qtvplugin_geomarker::CV_RectWrold2Mkt(QRectF world)
{
    if (!m_pVi)	return QRectF();
    double mx1,my1,mx2,my2;
    m_pVi->CV_World2MK(world.left(),world.top(),&mx1,&my1);
    m_pVi->CV_World2MK(world.right(),world.bottom(),&mx2,&my2);
    return QRectF(QPointF(mx1,my1),QPointF(mx2,my2));
}
QRectF qtvplugin_geomarker::current_sel_RectWorld()
{
    if (!m_pVi)	return QRectF();
    if (m_currentTools!=qtvplugin_geomarker::TOOLS_RECT_SELECTION)
	return QRectF();
    if (m_sel_ptEnd_World.isNull() || m_sel_ptStart_World.isNull())
	return QRectF();
    qint32 wsz = 256*(1<<m_pVi->level());
    int wx1 = m_sel_ptStart_World.x(),wx2 = m_sel_ptEnd_World.x(),
	    wy1 = m_sel_ptStart_World.y(), wy2 = m_sel_ptEnd_World.y();
    while (wx2 - wx1 > wsz/2)
	wx1+=wsz;
    while (wx2 - wx1 < -wsz/2)
	wx2+=wsz;
    if (wx1 > wx2)
    {
	float tp = wx1;
	wx1 = wx2;
	wx2 = tp;
    }
    if (wy1 > wy2)
    {
	float tp = wy1;
	wy1 = wy2;
	wy2 = tp;
    }

    return QRectF(QPointF(wx1,wy1),QPointF(wx2,wy2));
}

void qtvplugin_geomarker::clearSelection()
{
    if (!m_pVi)
	return ;
    foreach (QString name, m_set_itemNameSelected)
    {
	QTVP_GEOMARKER::geoItemBase * it = m_pScene->geoitem_by_name(name);
	if (it)
	    it->set_selected(false);
    }

    m_set_itemNameSelected.clear();
    refresh_selection_listview();
    m_pVi->UpdateWindow();
}
void qtvplugin_geomarker::addSelection(QRectF rectWorld)
{
    qint32 wsz = 256*(1<<m_pVi->level());
    bool changed = false;
    for (int i=-1;i<=1;++i)
    {
	double	x1 = rectWorld.left()+i * wsz,
		y1 = rectWorld.top(),
		x2 = rectWorld.right()+i * wsz,
		y2 = rectWorld.bottom();

	QList<QGraphicsItem *> itmesel = m_pScene->items(QRectF(QPointF(x1,y1),QPointF(x2,y2)));
	foreach (QGraphicsItem * it, itmesel)
	{
	    QTVP_GEOMARKER::geoItemBase *
		    gi = dynamic_cast<QTVP_GEOMARKER::geoItemBase *>(it);
	    if (gi)
	    {
		changed = true;
		QString nm = gi->item_name();
		if (m_set_itemNameSelected.contains(nm))
		{
		    m_set_itemNameSelected.remove(nm);
		    gi->set_selected(false);
		}
		else
		{
		    m_set_itemNameSelected.insert(nm);
		    gi->set_selected(true);
		}

	    }
	}
    }
    if (changed)
    {
	refresh_selection_listview();
	scheduleUpdateMap();
    }

}

bool qtvplugin_geomarker::cb_event(const QMap<QString, QVariant> para)
{
    if (para["source"]==this->get_name() && para["name"]=="ITEM_MOUSE_ENTER")
    {
	if (ui->checkBox_QTV_hoverEvt_AutoLabel->isChecked())
	{
	    const QString key = para["id"].toString();
	    if (key.length())
	    {
		QTVP_GEOMARKER::geoItemBase * base = m_pScene->geoitem_by_name(key);
		if (base)
		{
		    base->show_props(true);
		    scheduleUpdateMap();
		}
	    }
	}
    }
    if (para["source"]==this->get_name() && para["name"]=="ITEM_MOUSE_LEAVE")
    {
	if (ui->checkBox_QTV_hoverEvt_AutoLabel->isChecked())
	{
	    const QString key = para["id"].toString();
	    if (key.length())
	    {
		QTVP_GEOMARKER::geoItemBase * base = m_pScene->geoitem_by_name(key);
		if (base)
		{
		    base->show_props(false);
		    scheduleUpdateMap();
		}
	    }
	}
    }	return false;
}

/*! qtvplugin_geomarker::cb_mouseXXXEvent tranfer mouse events from main view to
 * QGraphicsItem based classes, so that these items will recieve mouse events.
 * For qt's graphics-view framework, this approach is done inside QGraphicsView class.
 * however, our main view is a simple widget, which means mouse events should be dealed manually.
 *
 * @fn qtvplugin_geomarker::cb_mouseDoubleClickEvent(QMouseEvent * e)
 * @param e the mouse event.
 * @return bool event acception.
*/
bool		qtvplugin_geomarker::cb_mouseDoubleClickEvent(QMouseEvent * e)
{
    if (!m_pVi)
	return false;
    if (m_bVisible==false)
	return false;
    QPoint mouse_view_pt = e->pos();
    int winsz = 256 * (1<<m_pVi->level());
    double wx,wy;
    double mlat, mlon;
    m_pVi->CV_DP2World(mouse_view_pt.x(),mouse_view_pt.y(),&wx,&wy);
    m_pVi->CV_DP2LLA(mouse_view_pt.x(),mouse_view_pt.y(),&mlat,&mlon);
    if (e->button()==Qt::RightButton)
    {
	ui->lineEdit_QTV_point_lat->setText(QString("%1").arg(mlat,0,'f',7));
	ui->lineEdit_QTV_point_lon->setText(QString("%1").arg(mlon,0,'f',7));
	ui->lineEdit_QTV_icon_lat->setText(QString("%1").arg(mlat,0,'f',7));
	ui->lineEdit_QTV_icon_lon->setText(QString("%1").arg(mlon,0,'f',7));
    }
    //Warp
    while (wx < 0) wx += winsz;
    while (wx > winsz-1) wx -= winsz;

    QPointF mouse_scene_pt(wx,wy);
    QPoint mouse_screen_pt = e->globalPos();
    Qt::MouseButton mouse_button = e->button();
    QWidget * pwig = dynamic_cast<QWidget *> (m_pVi);
    if (m_bVisible && pwig && too_many_items()==false)
    {
	// Convert and deliver the mouse event to the scene.
	QGraphicsSceneMouseEvent * pmouseEvent = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseDoubleClick);
	QGraphicsSceneMouseEvent & mouseEvent = * pmouseEvent;
	mouseEvent.setWidget(pwig);
	mouseEvent.setButtonDownScenePos(mouse_button, mouse_scene_pt);
	mouseEvent.setButtonDownScreenPos(mouse_button, mouse_screen_pt);
	mouseEvent.setScenePos(mouse_scene_pt);
	mouseEvent.setScreenPos(mouse_screen_pt);
	mouseEvent.setLastScenePos(mouse_scene_pt);
	mouseEvent.setLastScreenPos(mouse_screen_pt);
	mouseEvent.setButtons(e->buttons());
	mouseEvent.setButton(e->button());
	mouseEvent.setModifiers(e->modifiers());
	mouseEvent.setAccepted(false);
	QApplication::postEvent(m_pScene, &mouseEvent);
	scheduleUpdateMap();
    }
    return false;
}
/*! qtvplugin_geomarker::cb_mouseXXXEvent tranfer mouse events from main view to
 * QGraphicsItem based classes, so that these items will recieve mouse events.
 * For qt's graphics-view framework, this approach is done inside QGraphicsView class.
 * however, our main view is a simple widget, which means mouse events should be dealed manually.
 *
 * @fn qtvplugin_geomarker::cb_mousePressEvent(QMouseEvent * e)
 * @param e the mouse event.
 * @return bool event acception.
*/
bool qtvplugin_geomarker::cb_mousePressEvent(QMouseEvent * e)
{
    if (!m_pVi)
	return false;
    if (m_bVisible==false)
	return false;
    QPoint mouse_view_pt = e->pos();
    int winsz = 256 * (1<<m_pVi->level());
    double wx,wy;
    m_pVi->CV_DP2World(mouse_view_pt.x(),mouse_view_pt.y(),&wx,&wy);
    //Warp
    while (wx < 0) wx += winsz;
    while (wx > winsz-1) wx -= winsz;

    QPointF mouse_scene_pt(wx,wy);
    QPoint mouse_screen_pt = e->globalPos();
    Qt::MouseButton mouse_button = e->button();
    QWidget * pwig = dynamic_cast<QWidget *> (m_pVi);
    if (m_bVisible && pwig  && too_many_items()==false)
    {
	// Convert and deliver the mouse event to the scene.
	QGraphicsSceneMouseEvent * pmouseEvent = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
	QGraphicsSceneMouseEvent & mouseEvent = * pmouseEvent;
	mouseEvent.setWidget(pwig);
	mouseEvent.setButtonDownScenePos(mouse_button, mouse_scene_pt);
	mouseEvent.setButtonDownScreenPos(mouse_button, mouse_screen_pt);
	mouseEvent.setScenePos(mouse_scene_pt);
	mouseEvent.setScreenPos(mouse_screen_pt);
	mouseEvent.setLastScenePos(mouse_scene_pt);
	mouseEvent.setLastScreenPos(mouse_screen_pt);
	mouseEvent.setButtons(e->buttons());
	mouseEvent.setButton(e->button());
	mouseEvent.setModifiers(e->modifiers());
	mouseEvent.setAccepted(false);
	QApplication::postEvent(m_pScene, &mouseEvent);
	scheduleUpdateMap();
    }
    //tools
    switch (m_currentTools)
    {
    case qtvplugin_geomarker::TOOLS_RECT_SELECTION:
    {
	if (e->button()==Qt::LeftButton)
	{
	    m_sel_ptStart_World = m_sel_ptEnd_World = QPointF(wx,wy);
	}
    }
	break;
    default:
	break;
    }


    return false;

}
bool qtvplugin_geomarker::cb_mouseMoveEvent ( QMouseEvent * e )
{
    if (!m_pVi)
	return false;
    if (m_bVisible==false)
	return false;
    QPoint mouse_view_pt = e->pos();
    int winsz = 256 * (1<<m_pVi->level());
    double wx,wy;
    m_pVi->CV_DP2World(mouse_view_pt.x(),mouse_view_pt.y(),&wx,&wy);
    //Warp
    while (wx < 0) wx += winsz;
    while (wx > winsz-1) wx -= winsz;
    QPointF mouse_scene_pt(wx,wy);
    QPoint mouse_screen_pt = e->globalPos();
    Qt::MouseButton mouse_button = e->button();
    QWidget * pwig = dynamic_cast<QWidget *> (m_pVi);
    if (m_bVisible && pwig  && too_many_items()==false)
    {
	// Convert and deliver the mouse event to the scene.
	QGraphicsSceneMouseEvent * pmouseEvent = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseMove);
	QGraphicsSceneMouseEvent & mouseEvent = * pmouseEvent;
	mouseEvent.setWidget(pwig);
	mouseEvent.setButtonDownScenePos(mouse_button, mouse_scene_pt);
	mouseEvent.setButtonDownScreenPos(mouse_button, mouse_screen_pt);
	mouseEvent.setScenePos(mouse_scene_pt);
	mouseEvent.setScreenPos(mouse_screen_pt);
	mouseEvent.setLastScenePos(mouse_scene_pt);
	mouseEvent.setLastScreenPos(mouse_screen_pt);
	mouseEvent.setButtons(e->buttons());
	mouseEvent.setButton(e->button());
	mouseEvent.setModifiers(e->modifiers());
	mouseEvent.setAccepted(false);
	QApplication::postEvent(m_pScene, &mouseEvent);
    }
    //tools
    switch (m_currentTools)
    {
    case qtvplugin_geomarker::TOOLS_RECT_SELECTION:
    {
	if (e->buttons()==Qt::LeftButton)
	{
	    m_sel_ptEnd_World = QPointF(wx,wy);
	    scheduleUpdateMap();
	}
    }
	break;
    default:
	break;
    }


    return false;

}

bool qtvplugin_geomarker::cb_mouseReleaseEvent ( QMouseEvent * e )
{
    if (!m_pVi)
	return false;
    if (m_bVisible==false)
	return false;
    QPoint mouse_view_pt = e->pos();
    int winsz = 256 * (1<<m_pVi->level());
    double wx,wy;
    m_pVi->CV_DP2World(mouse_view_pt.x(),mouse_view_pt.y(),&wx,&wy);
    //Warp
    while (wx < 0) wx += winsz;
    while (wx > winsz-1) wx -= winsz;

    //tools
    switch (m_currentTools)
    {
    case qtvplugin_geomarker::TOOLS_RECT_SELECTION:
    {
	if (e->button()==Qt::LeftButton)
	{
	    m_sel_ptEnd_World = QPointF(wx,wy);
	    QRectF rectSel = current_sel_RectWorld();
	    m_sel_ptStart_World = m_sel_ptEnd_World = QPointF();
	    addSelection(rectSel);
	    scheduleUpdateMap();
	}
    }
	break;
    default:
	break;
    }


    return false;

}

/*! for convenience, color is stored in plain text in XML and UI.
 * the plain text color is 4 sub value , which stands for r,g,b,alpha.
 *
 * @fn qtvplugin_geomarker::string2color(const QString & s)
 * @param s the string color.
 * @return QColor is the color object.
*/
QColor qtvplugin_geomarker::string2color(const QString & s)
{
    QStringList lst = s.split(",",QString::SkipEmptyParts);
    int r = 255,g = 255, b = 255, a= 128;
    if (lst.empty()==false) {r = lst.first().toInt(); lst.pop_front();}
    if (lst.empty()==false) {g = lst.first().toInt(); lst.pop_front();}
    if (lst.empty()==false) {b = lst.first().toInt(); lst.pop_front();}
    if (lst.empty()==false) {a = lst.first().toInt(); lst.pop_front();}
    return QColor(r,g,b,a);
}
/*! for convenience, color is stored in plain text in XML and UI.
 * the plain text color is 4 sub value , which stands for r,g,b,alpha.
 *
 * @fn qtvplugin_geomarker::color2string(const QColor & col)
 * @param col the  color object.
 * @return QString is the color string.
*/
QString qtvplugin_geomarker::color2string(const QColor & col)
{
    QString str = QString("%1,%2,%3,%4").arg(col.red()).arg(col.green()).arg(col.blue()).arg(col.alpha());
    return str;
}

QString qtvplugin_geomarker::ini_file()
{
    if (m_SLLibPath.size())
	return m_SLLibPath + QString("%1").arg(m_nInstance) + ".ini";
    else
	return QCoreApplication::applicationFilePath() + QString("/geomarker%1.ini").arg(m_nInstance);
}

void qtvplugin_geomarker::scheduleRefreshMarks()
{
    if (!m_pVi || !m_pScene)
	return;
    //We do not refresh UI immediately after each mark-insert, for these inserts is very dense ,
    //BAD performence will arise if so.
    //We will set a flag and refresh the ui in timerEvent Instead.
    m_bNeedRefresh = true;
    m_items_to_insert.clear();
}
void qtvplugin_geomarker::scheduleUpdateMap()
{
    if (!m_pVi || !m_pScene)
	return;
    //We do not refresh MAP immediately after each mark-insert, for these inserts is very dense ,
    //BAD performence will arise if so.
    //We will set a flag and refresh the ui in timerEvent Instead.
    m_bNeedUpdateView = true;
}
void qtvplugin_geomarker::refresh_selection_listview()
{
    if (!m_pVi || !m_pScene)
	return;
    //refersh
    this->m_pSelItemNameModel->removeRows(0,this->m_pSelItemNameModel->rowCount());
    foreach (QString name, m_set_itemNameSelected)
    {
	m_pSelItemNameModel->appendRow(new QStandardItem(name));
    }
}

QTVP_GEOMARKER::geoItemBase *  qtvplugin_geomarker::update_line(const QString & name,double lat1, double lon1,double lat2, double lon2, QPen pen)
{
    QTVP_GEOMARKER::geoItemBase *  res = 0;
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
    QTVP_GEOMARKER::geoGraphicsLineItem * pitem = base?dynamic_cast<QTVP_GEOMARKER::geoGraphicsLineItem  *>(base):0;
    if (!pitem)
	pitem	= new QTVP_GEOMARKER::geoGraphicsLineItem(name,
							  this->m_pVi,
							  lat1,lon1,lat2,lon2);

    pitem->setPen(pen);

    if (base == pitem)
    {
	pitem->setGeo(lat1,lon1,lat2,lon2);
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


QTVP_GEOMARKER::geoItemBase *   qtvplugin_geomarker::update_polygon		(const QString & name,const QPolygonF latlons, QPen pen, QBrush brush, bool tp)
{
    QTVP_GEOMARKER::geoItemBase *  res = 0;
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
    if (tp==false)
    {
	//type convertion to T
	QTVP_GEOMARKER::geoGraphicsPolygonItem * pitem = base?dynamic_cast<QTVP_GEOMARKER::geoGraphicsPolygonItem  *>(base):0;
	if (!pitem)
	    pitem	= new QTVP_GEOMARKER::geoGraphicsPolygonItem(name,
								     this->m_pVi,
								     latlons);
	pitem->setPen(pen);
	pitem->setBrush(brush);
	if (base == pitem)
	{
	    pitem->setGeo(latlons);
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
    }
    else
    {
	//type convertion to T
	QTVP_GEOMARKER::geoGraphicsMultilineItem * pitem = base?dynamic_cast<QTVP_GEOMARKER::geoGraphicsMultilineItem  *>(base):0;
	if (!pitem)
	    pitem	= new QTVP_GEOMARKER::geoGraphicsMultilineItem(name,
								       this->m_pVi,
								       latlons);
	pitem->setPen(pen);
	//pitem->setBrush(QBrush(Qt::NoBrush));
	if (base == pitem)
	{
	    pitem->setGeo(latlons);
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
    }


    return res;
}
QTVP_GEOMARKER::geoItemBase *	qtvplugin_geomarker::update_icon(const QString & name,double lat, double lon,qreal scale, qreal rotate,int smooth, QString id)
{
    QTVP_GEOMARKER::geoItemBase *  res = 0;
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
    QTVP_GEOMARKER::geoGraphicsPixmapItem * pitem = base?dynamic_cast<QTVP_GEOMARKER::geoGraphicsPixmapItem  *>(base):0;
    if (!pitem)
    {
	const QTVP_GEOMARKER::tag_icon * iconp = 0;
	if (m_map_icons.contains(id))
	    iconp = &m_map_icons[id];
	else
	    iconp = &m_map_icons["default"];

	pitem	= new QTVP_GEOMARKER::geoGraphicsPixmapItem(name,this->m_pVi,
							    iconp,
							    lat,lon
							    );
    }
    if (base == pitem)
    {
	pitem->setGeo(lat,lon);
	if (m_map_icons.contains(id))
	    pitem->setPixmap(m_map_icons[id]);
	else
	    pitem->setPixmap(m_map_icons["default"]);
	res = pitem;
    }
    else if (false==this->m_pScene->addItem(pitem,0))
    {
	if (base != pitem)
	{
	    delete pitem;
	    pitem = 0;
	}
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
    if (pitem)
    {
	pitem->setScale(scale);
	pitem->setRotation(rotate);
	if (smooth)
	    pitem->setTransformationMode(Qt::SmoothTransformation);
	else
	    pitem->setTransformationMode(Qt::FastTransformation);
	pitem->adjustLabelPos();
    }
    return res;
}
bool qtvplugin_geomarker::cmd_save (QString cmdFile)
{
    QFile fp(cmdFile);
    if (fp.open(QIODevice::WriteOnly)==false)
	return false;
    QTextStream stream_out(&fp);
    //1. for each mark, write a root element
    QList<QTVP_GEOMARKER::geoItemBase *> items = m_pScene->geo_items();
    foreach (QTVP_GEOMARKER::geoItemBase * item, items)
    {
	QString cmd_str;
	QTVP_GEOMARKER::geo_item_type x_tp = item->item_type();
	QString x_name = item->item_name();

	//1.1. Mark
	switch (x_tp)
	{
	case QTVP_GEOMARKER::ITEAMTYPE_RECT_POINT:
	{
	    QTVP_GEOMARKER::geoGraphicsRectItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsRectItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_point;name="+x_name+QString(";type=%1;").arg(x_tp);
		cmd_str += QString("lat=%1;lon=%2;").arg(pU->lat(),0,'f',7).arg(pU->lon(),0,'f',7);
		//1.2 style
		cmd_str += "width="+ QString("%1;").arg(pU->width());
		cmd_str += "height="+ QString("%1;").arg(pU->height());
		cmd_str += "color_pen="+ color2string(pU->pen().color())+";";
		cmd_str += "style_pen="+ QString("%1;").arg(int(pU->pen().style()));
		cmd_str += "width_pen="+ QString("%1;").arg(int(pU->pen().width()));
		cmd_str += "color_brush="+ color2string(pU->brush().color())+";";
		cmd_str += "style_brush="+ QString("%1;").arg(int(pU->brush().style()));
	    }
	}
	    break;
	case QTVP_GEOMARKER::ITEAMTYPE_ELLIPSE_POINT:
	{
	    QTVP_GEOMARKER::geoGraphicsEllipseItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsEllipseItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_point;name="+x_name+QString(";type=%1;").arg(x_tp);
		cmd_str += QString("lat=%1;lon=%2;").arg(pU->lat(),0,'f',7).arg(pU->lon(),0,'f',7);
		//1.2 style
		cmd_str += "width="+ QString("%1;").arg(pU->width());
		cmd_str += "height="+ QString("%1;").arg(pU->height());
		cmd_str += "color_pen="+ color2string(pU->pen().color())+";";
		cmd_str += "style_pen="+ QString("%1;").arg(int(pU->pen().style()));
		cmd_str += "width_pen="+ QString("%1;").arg(int(pU->pen().width()));
		cmd_str += "color_brush="+ color2string(pU->brush().color())+";";
		cmd_str += "style_brush="+ QString("%1;").arg(int(pU->brush().style()));
	    }
	}
	    break;
	case QTVP_GEOMARKER::ITEAMTYPE_LINE:
	{
	    QTVP_GEOMARKER::geoGraphicsLineItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsLineItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_line;name="+x_name+QString(";type=%1;").arg(x_tp);
		cmd_str += QString("lat0=%1;lon0=%2;").arg(pU->lat1(),0,'f',7).arg(pU->lon1(),0,'f',7);
		cmd_str += QString("lat1=%1;lon1=%2;").arg(pU->lat2(),0,'f',7).arg(pU->lon2(),0,'f',7);
		//1.2 style
		cmd_str += "color_pen=" + color2string(pU->pen().color())+";";
		cmd_str += "style_pen=" + QString("%1;").arg(int(pU->pen().style()));
		cmd_str += "width_pen=" + QString("%1;").arg(int(pU->pen().width()));
	    }
	}
	    break;
	case QTVP_GEOMARKER::ITEAMTYPE_POLYGON:
	{
	    QTVP_GEOMARKER::geoGraphicsPolygonItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsPolygonItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_polygon;name="+x_name+QString(";type=%1;").arg(x_tp);
		QPolygonF pl = pU->llas();
		int nPl = pl.size();
		int ct_pg = 0;
		foreach (QPointF pf, pl)
		{
		    cmd_str += QString("lat%3=%1;lon%3=%2;").arg(pf.y(),0,'f',7).arg(pf.x(),0,'f',7).arg(ct_pg);
		    ++ct_pg;
		}
		cmd_str += "color_pen=" + color2string(pU->pen().color())+";";
		cmd_str += "style_pen=" + QString("%1;").arg(int(pU->pen().style()));
		cmd_str += "width_pen=" + QString("%1;").arg(int(pU->pen().width()));
		cmd_str += "color_brush=" + color2string(pU->brush().color())+";";
		cmd_str += "style_brush=" + QString("%1;").arg(int(pU->brush().style()));
	    }
	}
	    break;
	case QTVP_GEOMARKER::ITEAMTYPE_PIXMAP:
	{
	    QTVP_GEOMARKER::geoGraphicsPixmapItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsPixmapItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_icon;name="+x_name+QString(";type=%1;").arg(x_tp);
		//1.2. geo
		cmd_str += QString("lat=%1;lon=%2;").arg(pU->lat(),0,'f',7).arg(pU->lon(),0,'f',7);
		//1.2 style
		cmd_str += "icon=" +QString("%1;").arg(pU->icon()->name);
		cmd_str += "scale=" +QString("%1;").arg(pU->scale());
		cmd_str += "rotate=" +QString("%1;").arg(pU->rotation());
		cmd_str += "smooth=" +QString("%1;").arg(pU->transformationMode()==Qt::SmoothTransformation?1:0);
	    }
	}
	    break;
	case QTVP_GEOMARKER::ITEAMTYPE_MULTILINE:
	{
	    QTVP_GEOMARKER::geoGraphicsMultilineItem * pU = dynamic_cast<QTVP_GEOMARKER::geoGraphicsMultilineItem *>(item);
	    if (pU)
	    {
		cmd_str += "function=update_polygon;name="+x_name+QString(";type=%1;").arg(x_tp);
		QPolygonF pl = pU->llas();
		int nPl = pl.size();
		int ct_pg = 0;
		foreach (QPointF pf, pl)
		{
		    cmd_str += QString("lat%3=%1;lon%3=%2;").arg(pf.y(),0,'f',7).arg(pf.x(),0,'f',7).arg(ct_pg);
		    ++ct_pg;
		}
		cmd_str += "color_pen=" + color2string(pU->pen().color())+";";
		cmd_str += "style_pen=" + QString("%1;").arg(int(pU->pen().style()));
		cmd_str += "width_pen=" + QString("%1;").arg(int(pU->pen().width()));
		cmd_str += "color_brush=" + color2string(pU->brush().color())+";";
		cmd_str += "style_brush=" + QString("%1;").arg(int(pU->brush().style()));
	    }
	}
	    break;
	default:
	    break;
	}

	QColor colorText = item->labelColor();
	cmd_str += "color_label="+color2string(colorText)+";";

	int fsize = item->labelFont().pointSize();
	cmd_str += "size_label="+QString("%1;").arg(fsize);

	int weight = item->labelFont().weight();
	cmd_str += "weight_label="+QString("%1;").arg(weight);
	cmd_str += "want_hover="+QString("%1;\015\012").arg(item->wantMouseHoverEvent()==true?1:0);

	cmd_str += "function=update_props;name="+x_name+";";
	//1.2 properties
	int props = item->prop_counts();
	QStringList lstNames = item->prop_names();
	QVariantList lstValues = item->prop_values();
	for (int i=0;i<props;++i)
	{
	    cmd_str += lstNames[i]+"="+lstValues[i].toString()+";";
	}
	stream_out<<cmd_str<<"\015\012";
    }
    fp.flush();
    fp.close();
    return true;
}
bool qtvplugin_geomarker::cmd_load (QString cmdFile)
{
    bool res = true;
    QString errMessage;
    QFile fp(cmdFile);
    if (fp.open(QIODevice::ReadOnly)==false)
	return false;
    QTextStream sfile(&fp);
    while (sfile.atEnd()==false)
    {
	QString linered = sfile.readLine();
	if (linered.contains("function"))
	{
	    QMap<QString, QVariant> res;
	    QStringList lst = linered.split(";");
	    foreach (QString s, lst)
	    {
		int t = s.indexOf("=");
		if (t>0 && t< s.size())
		{
		    QString name = s.left(t).trimmed();
		    QString value = s.mid(t+1).trimmed();
		    res[name] = value;
		}
	    }
	    this->call_func(res);
	}
    }
    fp.close();
    if (res==false)
    {
	QMap<QString,QVariant> evt_error;
	evt_error["source"] = get_name();
	evt_error["destin"] = "ALL";
	evt_error["name"] = "error";
	evt_error["class"] = "cmd reader";
	evt_error["file"] = cmdFile;
	evt_error["detail"] = errMessage;
	m_pVi->post_event(evt_error);

    }
    return res;
}
