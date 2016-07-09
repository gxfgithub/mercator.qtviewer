#include "qtvplugin_geomarker.h"
#include "ui_qtvplugin_geomarker.h"
#include <QSettings>
#include <QColorDialog>
#include <QSet>
#include <QMap>
#include <QRegExp>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "geographicsellipseitem.h"
#include "geographicsrectitem.h"
#include "geographicslineitem.h"
#include "geographicspolygonitem.h"
#include "geographicsmultilineitem.h"
#include "dialogselecticon.h"

void		qtvplugin_geomarker::timerEvent(QTimerEvent * e)
{
	if (e->timerId()==m_nTimerID_refreshUI && m_bNeedRefresh)
	{
		killTimer(m_nTimerID_refreshUI);
		m_nTimerID_refreshUI = -1;
		if (m_items_to_insert.empty()==true)
		{
			m_items_to_insert = m_pScene->geo_item_names();
			m_pGeoItemModel->removeRows(0,m_pGeoItemModel->rowCount());
		}
		//refersh
		int ct = 0;
		//we do not use geoItemsBase pointer, because a geoItemsBase pointer may be invalid when delayed ipdate approach is taking place.
		while (++ct < 2048 && m_items_to_insert.empty()==false)
		{
			QString keyname = m_items_to_insert.first();
			m_items_to_insert.pop_front();
			QTVP_GEOMARKER::geoItemBase * item = m_pScene->geoitem_by_name(keyname);
			if (!item)
				continue;
			int c = m_pGeoItemModel->rowCount();
			m_pGeoItemModel->appendRow(new QStandardItem(item->item_name()));
			m_pGeoItemModel->setData(
						m_pGeoItemModel->index(c,1),
						QTVP_GEOMARKER::item_name_by_enum(item->item_type())
						);
			m_pGeoItemModel->setData(
						m_pGeoItemModel->index(c,2),
						item->prop_counts()
						);

		}
		if (m_items_to_insert.empty()==true)
			m_bNeedRefresh = false;
		m_nTimerID_refreshUI = startTimer(217);
	}
	else if (m_nTimerID_refreshMap==e->timerId() && m_bNeedUpdateView==true)
	{
		m_bNeedUpdateView = false;
		killTimer(m_nTimerID_refreshMap);
		m_nTimerID_refreshMap = -1;
		m_pVi->UpdateWindow();
		m_nTimerID_refreshMap = startTimer(97);
	}
	else if (m_nTimerID_levelQueue == e->timerId())
	{
		killTimer(m_nTimerID_levelQueue);
		++m_nDivideTimer;
		if (true==m_pScene->deal_level_queue())
			m_pVi->UpdateWindow();
		else if (m_nDivideTimer % 20 == 0)
		{
			if (m_pScene->progress_queue()<0.999)
				m_pVi->UpdateWindow();
		}
		m_nTimerID_levelQueue = startTimer(119);
	}
}




void qtvplugin_geomarker::on_radioButton_QTV_tool_point_toggled(bool b)
{
	if (b) ui->stackedWidget_marks->setCurrentIndex(0);
}

void qtvplugin_geomarker::on_radioButton_QTV_tool_line_toggled(bool b )
{
	if (b) ui->stackedWidget_marks->setCurrentIndex(1);
}

void qtvplugin_geomarker::on_radioButton_QTV_tool_polygon_toggled(bool b)
{
	if (b) ui->stackedWidget_marks->setCurrentIndex(2);
}
void qtvplugin_geomarker::on_radioButton_QTV_tool_icons_toggled(bool b )
{
	if (b) ui->stackedWidget_marks->setCurrentIndex(3);
}

void qtvplugin_geomarker::on_toolButton_QTV_selColorPen_clicked()
{
	QString str = ui->lineEdit_QTV_PenColor->text();

	QColor col = QColorDialog::getColor(string2color(str),this,tr("Select Color"),QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if (col.isValid())
		ui->lineEdit_QTV_PenColor->setText(color2string(col));
}
void qtvplugin_geomarker::on_toolButton_QTV_selColorFill_clicked()
{
	QString str = ui->lineEdit_QTV_FillColor->text();

	QColor col = QColorDialog::getColor(string2color(str),this,tr("Select Color"),QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if (col.isValid())
		ui->lineEdit_QTV_FillColor->setText(color2string(col));
}
void qtvplugin_geomarker::on_toolButton_QTV_selColorText_clicked()
{
	QString str = ui->lineEdit_QTV_TextColor->text();

	QColor col = QColorDialog::getColor(string2color(str),this,tr("Select Color"),QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog);
	if (col.isValid())
		ui->lineEdit_QTV_TextColor->setText(color2string(col));
}
void qtvplugin_geomarker::on_pushButton_QTV_pickToLine1_clicked()
{
	ui->lineEdit_QTV_lineLat1->setText(ui->lineEdit_QTV_point_lat->text());
	ui->lineEdit_QTV_lineLon1->setText(ui->lineEdit_QTV_point_lon->text());
}

void qtvplugin_geomarker::on_pushButton_QTV_pickToLine2_clicked()
{
	ui->lineEdit_QTV_lineLat2->setText(ui->lineEdit_QTV_point_lat->text());
	ui->lineEdit_QTV_lineLon2->setText(ui->lineEdit_QTV_point_lon->text());
}


void qtvplugin_geomarker::ini_save()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	int radioButton_QTV_tool_point = 0;
	if (ui->radioButton_QTV_tool_line->isChecked()==true) radioButton_QTV_tool_point = 1;
	else if (ui->radioButton_QTV_tool_polygon->isChecked()==true) radioButton_QTV_tool_point = 2;
	else if (ui->radioButton_QTV_tool_icons->isChecked()==true) radioButton_QTV_tool_point = 3;
	else radioButton_QTV_tool_point = 0;
	settings.setValue("ui/radioButton_QTV_tool_point",radioButton_QTV_tool_point);

	settings.setValue("ui/lineEdit_QTV_currentID",ui->lineEdit_QTV_currentID->text());

	settings.setValue("ui/lineEdit_QTV_point_lat",ui->lineEdit_QTV_point_lat->text());
	settings.setValue("ui/lineEdit_QTV_point_lon",ui->lineEdit_QTV_point_lon->text());
	settings.setValue("ui/spinBox_QTV_point_width",ui->spinBox_QTV_point_width->value());
	settings.setValue("ui/spinBox_QTV_point_height",ui->spinBox_QTV_point_height->value());


	int radioButton_QTV_PointRect = 0;
	if (ui->radioButton_QTV_PointRound->isChecked()==true) radioButton_QTV_PointRect = 1;
	else radioButton_QTV_PointRect = 0;
	settings.setValue("ui/radioButton_QTV_PointRect",radioButton_QTV_PointRect);

	settings.setValue("ui/lineEdit_QTV_PenColor",ui->lineEdit_QTV_PenColor->text());
	settings.setValue("ui/lineEdit_QTV_FillColor",ui->lineEdit_QTV_FillColor->text());
	settings.setValue("ui/lineEdit_QTV_TextColor",ui->lineEdit_QTV_TextColor->text());
	settings.setValue("ui/lineEdit_QTV_prop_name",ui->lineEdit_QTV_prop_name->text());
	settings.setValue("ui/lineEdit_QTV_prop_string",ui->lineEdit_QTV_prop_string->text());
	settings.setValue("ui/lineEdit_QTV_lineLat1",ui->lineEdit_QTV_lineLat1->text());
	settings.setValue("ui/lineEdit_QTV_lineLat2",ui->lineEdit_QTV_lineLat2->text());
	settings.setValue("ui/lineEdit_QTV_lineLon1",ui->lineEdit_QTV_lineLon1->text());
	settings.setValue("ui/lineEdit_QTV_lineLon2",ui->lineEdit_QTV_lineLon2->text());
	settings.setValue("ui/spinBox_QTV_penWidth",ui->spinBox_QTV_penWidth->value());
	settings.setValue("ui/comboBox_QTV_linePad",ui->comboBox_QTV_linePad->currentIndex());
	settings.setValue("ui/comboBox_QTV_fillPad",ui->comboBox_QTV_fillPad->currentIndex());
	settings.setValue("ui/plainTextEdit_QTV_corners",ui->plainTextEdit_QTV_corners->toPlainText());

	settings.setValue("ui/spinBox_QTV_textWeight",ui->spinBox_QTV_textWeight->value());
	settings.setValue("ui/spinBox_QTV_fontSize",ui->spinBox_QTV_fontSize->value());
	settings.setValue("ui/lineEdit_QTV_icon_lat",ui->lineEdit_QTV_icon_lat->text());
	settings.setValue("ui/lineEdit_QTV_icon_lon",ui->lineEdit_QTV_icon_lon->text());
	settings.setValue("ui/lineEdit_QTV_icon_rotate",ui->lineEdit_QTV_icon_rotate->text());
	settings.setValue("ui/lineEdit_QTV_icon_scale",ui->lineEdit_QTV_icon_scale->text());
	settings.setValue("ui/checkBox_QTV_multiline",ui->checkBox_QTV_multiline->isChecked()?-1:0);
}

void qtvplugin_geomarker::ini_load()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	int radioButton_QTV_tool_point =  settings.value("ui/radioButton_QTV_tool_point",0).toInt();
	switch (radioButton_QTV_tool_point)
	{
	case 0:
		ui->radioButton_QTV_tool_point->setChecked(true);
		break;
	case 1:
		ui->radioButton_QTV_tool_line->setChecked(true);
		break;
	case 2:
		ui->radioButton_QTV_tool_polygon->setChecked(true);
		break;
	case 3:
		ui->radioButton_QTV_tool_icons->setChecked(true);
		break;
	default:
		ui->radioButton_QTV_tool_point->setChecked(true);
		break;
	}
	QString lineEdit_QTV_currentID = settings.value("ui/lineEdit_QTV_currentID","PT0").toString();
	ui->lineEdit_QTV_currentID->setText(lineEdit_QTV_currentID);

	QString lineEdit_QTV_point_lat = settings.value("ui/lineEdit_QTV_point_lat","0").toString();
	ui->lineEdit_QTV_point_lat->setText(lineEdit_QTV_point_lat);
	QString lineEdit_QTV_point_lon = settings.value("ui/lineEdit_QTV_point_lon","0").toString();
	ui->lineEdit_QTV_point_lon->setText(lineEdit_QTV_point_lon);
	int spinBox_QTV_point_width = settings.value("ui/spinBox_QTV_point_width",8).toInt();
	ui->spinBox_QTV_point_width->setValue(spinBox_QTV_point_width);
	int spinBox_QTV_point_height = settings.value("ui/spinBox_QTV_point_height",8).toInt();
	ui->spinBox_QTV_point_height->setValue(spinBox_QTV_point_height);
	QString lineEdit_QTV_PenColor = settings.value("ui/lineEdit_QTV_PenColor",color2string(QColor(0,0,0,128))).toString();
	ui->lineEdit_QTV_PenColor->setText(lineEdit_QTV_PenColor);
	QString lineEdit_QTV_FillColor = settings.value("ui/lineEdit_QTV_FillColor",color2string(QColor(255,255,255,128))).toString();
	ui->lineEdit_QTV_FillColor->setText(lineEdit_QTV_FillColor);
	QString lineEdit_QTV_TextColor = settings.value("ui/lineEdit_QTV_TextColor",color2string(QColor(0,0,0))).toString();
	ui->lineEdit_QTV_TextColor->setText(lineEdit_QTV_TextColor);
	int radioButton_QTV_PointRect =  settings.value("ui/radioButton_QTV_PointRect",0).toInt();
	switch (radioButton_QTV_PointRect)
	{
	case 0:
		ui->radioButton_QTV_PointRect->setChecked(true);
		break;
	case 1:
		ui->radioButton_QTV_PointRound->setChecked(true);
		break;
	default:
		ui->radioButton_QTV_PointRect->setChecked(true);
		break;
	}

	QString lineEdit_QTV_prop_name = settings.value("ui/lineEdit_QTV_prop_name","LABEL").toString();
	ui->lineEdit_QTV_prop_name->setText(lineEdit_QTV_prop_name);
	QString lineEdit_QTV_prop_string = settings.value("ui/lineEdit_QTV_prop_string","please input label here").toString();
	ui->lineEdit_QTV_prop_string->setText(lineEdit_QTV_prop_string);
	QString lineEdit_QTV_lineLat1 = settings.value("ui/lineEdit_QTV_lineLat1","0").toString();
	ui->lineEdit_QTV_lineLat1->setText(lineEdit_QTV_lineLat1);
	QString lineEdit_QTV_lineLat2 = settings.value("ui/lineEdit_QTV_lineLat2","0").toString();
	ui->lineEdit_QTV_lineLat2->setText(lineEdit_QTV_lineLat2);
	QString lineEdit_QTV_lineLon1 = settings.value("ui/lineEdit_QTV_lineLon1","0").toString();
	ui->lineEdit_QTV_lineLon1->setText(lineEdit_QTV_lineLon1);
	QString lineEdit_QTV_lineLon2 = settings.value("ui/lineEdit_QTV_lineLon2","0").toString();
	ui->lineEdit_QTV_lineLon2->setText(lineEdit_QTV_lineLon2);
	int spinBox_QTV_penWidth = settings.value("ui/spinBox_QTV_penWidth",3).toInt();
	ui->spinBox_QTV_penWidth->setValue(spinBox_QTV_penWidth);
	int comboBox_QTV_linePad = settings.value("ui/comboBox_QTV_linePad",1).toInt();
	ui->comboBox_QTV_linePad->setCurrentIndex(comboBox_QTV_linePad);
	int comboBox_QTV_fillPad = settings.value("ui/comboBox_QTV_fillPad",1).toInt();
	ui->comboBox_QTV_fillPad->setCurrentIndex(comboBox_QTV_fillPad);

	QString plainTextEdit_QTV_corners = settings.value("ui/plainTextEdit_QTV_corners","0").toString();
	ui->plainTextEdit_QTV_corners->setPlainText(plainTextEdit_QTV_corners);

	int spinBox_QTV_fontSize = settings.value("ui/spinBox_QTV_fontSize",9).toInt();
	ui->spinBox_QTV_fontSize->setValue(spinBox_QTV_fontSize);

	int spinBox_QTV_textWeight = settings.value("ui/spinBox_QTV_textWeight",16).toInt();
	ui->spinBox_QTV_textWeight->setValue(spinBox_QTV_textWeight);

	QString lineEdit_QTV_icon_lat = settings.value("ui/lineEdit_QTV_icon_lat","0").toString();
	ui->lineEdit_QTV_icon_lat->setText(lineEdit_QTV_icon_lat);
	QString lineEdit_QTV_icon_lon = settings.value("ui/lineEdit_QTV_icon_lon","0").toString();
	ui->lineEdit_QTV_icon_lon->setText(lineEdit_QTV_icon_lon);
	QString lineEdit_QTV_icon_scale = settings.value("ui/lineEdit_QTV_icon_scale","1.0").toString();
	ui->lineEdit_QTV_icon_scale->setText(lineEdit_QTV_icon_scale);
	QString lineEdit_QTV_icon_rotate = settings.value("ui/lineEdit_QTV_icon_rotate","1.0").toString();
	ui->lineEdit_QTV_icon_rotate->setText(lineEdit_QTV_icon_rotate);

	int checkBox_QTV_multiline = settings.value("ui/checkBox_QTV_multiline",0).toInt();
	ui->checkBox_QTV_multiline->setChecked(checkBox_QTV_multiline?true:false);

}


void qtvplugin_geomarker::style_save()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	settings.setValue("default_style/spinBox_QTV_point_width",ui->spinBox_QTV_point_width->value());
	settings.setValue("default_style/spinBox_QTV_point_height",ui->spinBox_QTV_point_height->value());
	int radioButton_QTV_PointRect = 0;
	if (ui->radioButton_QTV_PointRound->isChecked()==true) radioButton_QTV_PointRect = 1;
	else radioButton_QTV_PointRect = 0;
	settings.setValue("default_style/radioButton_QTV_PointRect",radioButton_QTV_PointRect);

	settings.setValue("default_style/lineEdit_QTV_PenColor",ui->lineEdit_QTV_PenColor->text());
	settings.setValue("default_style/lineEdit_QTV_FillColor",ui->lineEdit_QTV_FillColor->text());
	settings.setValue("default_style/lineEdit_QTV_TextColor",ui->lineEdit_QTV_TextColor->text());
	settings.setValue("default_style/spinBox_QTV_penWidth",ui->spinBox_QTV_penWidth->value());
	settings.setValue("default_style/comboBox_QTV_linePad",ui->comboBox_QTV_linePad->currentIndex());
	settings.setValue("default_style/comboBox_QTV_fillPad",ui->comboBox_QTV_fillPad->currentIndex());

	settings.setValue("default_style/spinBox_QTV_textWeight",ui->spinBox_QTV_textWeight->value());
	settings.setValue("default_style/spinBox_QTV_fontSize",ui->spinBox_QTV_fontSize->value());
	settings.setValue("default_style/lineEdit_QTV_icon_rotate",ui->lineEdit_QTV_icon_rotate->text());
	settings.setValue("default_style/lineEdit_QTV_icon_scale",ui->lineEdit_QTV_icon_scale->text());
	settings.setValue("default_style/checkBox_QTV_multiline",ui->checkBox_QTV_multiline->isChecked()?-1:0);
	settings.setValue("default_style/checkBox_QTV_icon_smooth",ui->checkBox_QTV_icon_smooth->isChecked()?-1:0);
	settings.setValue("default_style/comboBox_QTV_icons",ui->comboBox_QTV_icons->currentText());

	//update current style
	m_default_style.n_point_width = ui->spinBox_QTV_point_width->value();
	m_default_style.n_point_height = ui->spinBox_QTV_point_height->value();
	m_default_style.n_point_rect = radioButton_QTV_PointRect;
	m_default_style.pen.setColor(string2color(ui->lineEdit_QTV_PenColor->text()));
	m_default_style.pen.setWidth(ui->spinBox_QTV_penWidth->value());
	m_default_style.pen.setStyle(static_cast<Qt::PenStyle>(ui->comboBox_QTV_linePad->currentIndex()));
	m_default_style.brush.setColor(string2color(ui->lineEdit_QTV_FillColor->text()));
	m_default_style.brush.setStyle(static_cast<Qt::BrushStyle>(ui->comboBox_QTV_fillPad->currentIndex()));
	m_default_style.text_color = string2color(ui->lineEdit_QTV_TextColor->text());
	m_default_style.font.setWeight(ui->spinBox_QTV_textWeight->value());
	m_default_style.font.setPointSize(ui->spinBox_QTV_fontSize->value());
	m_default_style.scale = ui->lineEdit_QTV_icon_scale->text().toFloat();
	m_default_style.rotate = ui->lineEdit_QTV_icon_rotate->text().toFloat();
	m_default_style.multiline = ui->checkBox_QTV_multiline->isChecked()?-1:0;
	m_default_style.smooth = ui->checkBox_QTV_icon_smooth->isChecked()?-1:0;
	m_default_style.icon_name = ui->comboBox_QTV_icons->currentText();
}

void qtvplugin_geomarker::style_load()
{
	QSettings settings(ini_file(),QSettings::IniFormat);

	int spinBox_QTV_point_width = settings.value("default_style/spinBox_QTV_point_width",8).toInt();
	ui->spinBox_QTV_point_width->setValue(spinBox_QTV_point_width);

	int spinBox_QTV_point_height = settings.value("default_style/spinBox_QTV_point_height",8).toInt();
	ui->spinBox_QTV_point_height->setValue(spinBox_QTV_point_height);

	QString lineEdit_QTV_PenColor = settings.value("default_style/lineEdit_QTV_PenColor",color2string(QColor(0,0,0,128))).toString();
	ui->lineEdit_QTV_PenColor->setText(lineEdit_QTV_PenColor);

	QString lineEdit_QTV_FillColor = settings.value("default_style/lineEdit_QTV_FillColor",color2string(QColor(255,255,255,128))).toString();
	ui->lineEdit_QTV_FillColor->setText(lineEdit_QTV_FillColor);

	QString lineEdit_QTV_TextColor = settings.value("default_style/lineEdit_QTV_TextColor",color2string(QColor(0,0,0))).toString();
	ui->lineEdit_QTV_TextColor->setText(lineEdit_QTV_TextColor);

	int radioButton_QTV_PointRect =  settings.value("default_style/radioButton_QTV_PointRect",0).toInt();
	switch (radioButton_QTV_PointRect)
	{
	case 0:
		ui->radioButton_QTV_PointRect->setChecked(true);
		break;
	case 1:
		ui->radioButton_QTV_PointRound->setChecked(true);
		break;
	default:
		ui->radioButton_QTV_PointRect->setChecked(true);
		break;
	}

	int spinBox_QTV_penWidth = settings.value("default_style/spinBox_QTV_penWidth",3).toInt();
	ui->spinBox_QTV_penWidth->setValue(spinBox_QTV_penWidth);

	int comboBox_QTV_linePad = settings.value("default_style/comboBox_QTV_linePad",1).toInt();
	ui->comboBox_QTV_linePad->setCurrentIndex(comboBox_QTV_linePad);

	int comboBox_QTV_fillPad = settings.value("default_style/comboBox_QTV_fillPad",1).toInt();
	ui->comboBox_QTV_fillPad->setCurrentIndex(comboBox_QTV_fillPad);


	int spinBox_QTV_fontSize = settings.value("default_style/spinBox_QTV_fontSize",9).toInt();
	ui->spinBox_QTV_fontSize->setValue(spinBox_QTV_fontSize);

	int spinBox_QTV_textWeight = settings.value("default_style/spinBox_QTV_textWeight",16).toInt();
	ui->spinBox_QTV_textWeight->setValue(spinBox_QTV_textWeight);

	QString lineEdit_QTV_icon_scale = settings.value("default_style/lineEdit_QTV_icon_scale","1.0").toString();
	ui->lineEdit_QTV_icon_scale->setText(lineEdit_QTV_icon_scale);

	QString lineEdit_QTV_icon_rotate = settings.value("default_style/lineEdit_QTV_icon_rotate","1.0").toString();
	ui->lineEdit_QTV_icon_rotate->setText(lineEdit_QTV_icon_rotate);

	int checkBox_QTV_multiline = settings.value("default_style/checkBox_QTV_multiline",0).toInt();
	ui->checkBox_QTV_multiline->setChecked(checkBox_QTV_multiline?true:false);

	int checkBox_QTV_icon_smooth = settings.value("default_style/checkBox_QTV_icon_smooth",0).toInt();
	ui->checkBox_QTV_icon_smooth->setChecked(checkBox_QTV_icon_smooth?true:false);

	QString comboBox_QTV_icons = settings.value("default_style/comboBox_QTV_icons","default").toString();
	if (m_map_icons.contains(comboBox_QTV_icons))
		ui->comboBox_QTV_icons->setCurrentText(comboBox_QTV_icons);
	//update current style
	m_default_style.n_point_width = ui->spinBox_QTV_point_width->value();
	m_default_style.n_point_height = ui->spinBox_QTV_point_height->value();
	m_default_style.n_point_rect = radioButton_QTV_PointRect;
	m_default_style.pen.setColor(string2color(ui->lineEdit_QTV_PenColor->text()));
	m_default_style.pen.setWidth(ui->spinBox_QTV_penWidth->value());
	m_default_style.pen.setStyle(static_cast<Qt::PenStyle>(ui->comboBox_QTV_linePad->currentIndex()));
	m_default_style.brush.setColor(string2color(ui->lineEdit_QTV_FillColor->text()));
	m_default_style.brush.setStyle(static_cast<Qt::BrushStyle>(ui->comboBox_QTV_fillPad->currentIndex()));
	m_default_style.text_color = string2color(ui->lineEdit_QTV_TextColor->text());
	m_default_style.font.setWeight(ui->spinBox_QTV_textWeight->value());
	m_default_style.font.setPointSize(ui->spinBox_QTV_fontSize->value());
	m_default_style.scale = ui->lineEdit_QTV_icon_scale->text().toFloat();
	m_default_style.rotate = ui->lineEdit_QTV_icon_rotate->text().toFloat();
	m_default_style.multiline = ui->checkBox_QTV_multiline->isChecked()?-1:0;
	m_default_style.smooth = ui->checkBox_QTV_icon_smooth->isChecked()?-1:0;
	m_default_style.icon_name = ui->comboBox_QTV_icons->currentText();
}

void qtvplugin_geomarker::on_pushButton_QTV_update_clicked()
{
	if (m_pVi==0 || !m_pScene)
		return;
	QString name = ui->lineEdit_QTV_currentID->text();
	ini_save();

	//Get pen and brush settings
	Qt::PenStyle pst [] ={
		Qt::NoPen	,
		Qt::SolidLine	,
		Qt::DashLine	,
		Qt::DotLine	,
		Qt::DashDotLine	,
		Qt::DashDotDotLine	,
		Qt::CustomDashLine
	};
	Qt::BrushStyle bst [] = {
		Qt::NoBrush,
		Qt::SolidPattern,
		Qt::Dense1Pattern,
		Qt::Dense2Pattern,
		Qt::Dense3Pattern,
		Qt::Dense4Pattern,
		Qt::Dense5Pattern,
		Qt::Dense6Pattern,
		Qt::Dense7Pattern,
		Qt::HorPattern,
		Qt::VerPattern,
		Qt::CrossPattern,
		Qt::BDiagPattern,
		Qt::FDiagPattern,
		Qt::DiagCrossPattern
	};

	int ptdd = ui->comboBox_QTV_linePad->currentIndex();
	if (ptdd < 0 || ptdd >=7)
		ptdd = 1;
	QColor penColor = string2color( ui->lineEdit_QTV_PenColor->text());
	int penWidth = ui->spinBox_QTV_penWidth->value();
	QPen pen;//(QBrush(color),width,pst[ptdd]);
	pen.setColor(penColor);
	pen.setWidth(penWidth);
	pen.setStyle(pst[ptdd]);

	int btdd = ui->comboBox_QTV_fillPad->currentIndex();
	if (btdd < 0 || btdd >=15)
		btdd = 1;

	QColor brushColor = string2color( ui->lineEdit_QTV_FillColor->text());
	QBrush brush;
	brush.setColor(brushColor);
	brush.setStyle(bst[btdd]);

	QTVP_GEOMARKER::geoItemBase * newitem = 0;

	if (ui->radioButton_QTV_tool_point->isChecked())
	{
		double lat = ui->lineEdit_QTV_point_lat->text().toDouble();
		double lon = ui->lineEdit_QTV_point_lon->text().toDouble();
		int tp = ui->radioButton_QTV_PointRect->isChecked()?0:1;
		int width = ui->spinBox_QTV_point_width->value();
		int height = ui->spinBox_QTV_point_height->value();
		if (tp==0)
			newitem = update_point<QTVP_GEOMARKER::geoGraphicsRectItem>(name,lat,lon,width,height,pen,brush);
		else
			newitem = update_point<QTVP_GEOMARKER::geoGraphicsEllipseItem>(name,lat,lon,width,height,pen,brush);
	}
	else if (ui->radioButton_QTV_tool_line->isChecked())
	{
		double lat1 = ui->lineEdit_QTV_lineLat1->text().toDouble();
		double lat2 = ui->lineEdit_QTV_lineLat2->text().toDouble();
		double lon1 = ui->lineEdit_QTV_lineLon1->text().toDouble();
		double lon2 = ui->lineEdit_QTV_lineLon2->text().toDouble();
		newitem = update_line(name,lat1,lon1,lat2,lon2,pen);
	}
	else if (ui->radioButton_QTV_tool_polygon->isChecked())
	{
		QPolygonF latlons;
		QString strPlainTexts = ui->plainTextEdit_QTV_corners->toPlainText();
		strPlainTexts.remove(' ');
		strPlainTexts.remove('\n');
		strPlainTexts.remove('\r');
		strPlainTexts.remove('\015');
		strPlainTexts.remove('\012');
		QStringList lst = strPlainTexts.split(QRegExp("[,;]"),QString::SkipEmptyParts);
		int c = 0;
		QPointF ll;
		foreach (QString s,lst)
		{
			if (c%2==0)
				ll.setY(s.toDouble());
			else
				ll.setX(s.toDouble());
			if ((++c) % 2==0)
				latlons.push_back(ll);
		}
		if (latlons.size())
			newitem = update_polygon(name,latlons,pen,brush,ui->checkBox_QTV_multiline->isChecked()?true:false);

	}
	else if (ui->radioButton_QTV_tool_icons->isChecked())
	{
		double lat = ui->lineEdit_QTV_icon_lat->text().toDouble();
		double lon = ui->lineEdit_QTV_icon_lon->text().toDouble();
		qreal scale = ui->lineEdit_QTV_icon_scale->text().toFloat();
		qreal rotate = ui->lineEdit_QTV_icon_rotate->text().toFloat();
		QString iconname = ui->comboBox_QTV_icons->currentText();
		int smooth = ui->checkBox_QTV_icon_smooth->isChecked()?1:0;
		newitem = update_icon(name,lat,lon,scale,rotate,smooth,iconname);
	}
	else
		return;
	if (newitem)
	{
		int fontSz = ui->spinBox_QTV_fontSize->value();
		int fontWeight = ui->spinBox_QTV_textWeight->value();
		QColor textColor = string2color( ui->lineEdit_QTV_TextColor->text());
		QFont f = newitem->labelFont();
		f.setPointSize(fontSz);
		f.setWeight(fontWeight);
		newitem->setLabelFont(f);
		newitem->setLabelColor(textColor);


	}
	scheduleRefreshMarks();
	m_pVi->UpdateWindow();
}

void qtvplugin_geomarker::on_pushButton_QTV_del_clicked()
{
	QSet<int> rows;
	QModelIndexList lst =  ui->tableView_QTV_marks->selectionModel()->selectedIndexes();
	foreach (QModelIndex idd, lst)
		rows.insert(idd.row());
	foreach (int row, rows)
	{
		QString namep = m_pGeoItemModel->data(m_pGeoItemModel->index(row,0)).toString();
		QTVP_GEOMARKER::geoItemBase * b = m_pScene->geoitem_by_name(namep);
		if (b)
			m_pScene->removeItem(b,0);
	}
	scheduleRefreshMarks();

}

void qtvplugin_geomarker::on_pushButton_QTV_prop_update_clicked()
{
	ini_save();
	QString name = ui->lineEdit_QTV_currentID->text();
	//Fill in the pages
	QTVP_GEOMARKER::geoItemBase * item = m_pScene->geoitem_by_name(name);
	if (item)
	{
		item->set_prop_data(ui->lineEdit_QTV_prop_name->text(),ui->lineEdit_QTV_prop_string->text());
		this->refreshProps(item);
		this->scheduleRefreshMarks();
		//Update Font
		int fontSz = ui->spinBox_QTV_fontSize->value();
		int fontWeight = ui->spinBox_QTV_textWeight->value();
		QColor textColor = string2color( ui->lineEdit_QTV_TextColor->text());
		QFont f = item->labelFont();
		f.setPointSize(fontSz);
		f.setWeight(fontWeight);
		item->setLabelFont(f);
		item->setLabelColor(textColor);

		m_pVi->UpdateWindow();
	}
}
void qtvplugin_geomarker::on_pushButton_QTV_prop_delete_clicked()
{
	QString name = ui->lineEdit_QTV_currentID->text();
	//Fill in the pages
	QTVP_GEOMARKER::geoItemBase * item = m_pScene->geoitem_by_name(name);
	if (item)
	{
		QSet<int> rows;
		QModelIndexList lst =  ui->tableView_QTV_props->selectionModel()->selectedIndexes();
		foreach (QModelIndex idd, lst)
			rows.insert(idd.row());
		foreach (int row, rows)
		{
			QString namep = m_pGeoPropModel->data(m_pGeoPropModel->index(row,0)).toString();
			item->del_prop(namep);
		}
	}
	this->refreshProps(item);
}


void qtvplugin_geomarker::on_tableView_QTV_marks_doubleClicked(const QModelIndex & index)
{
	int row = index.row();
	if (row >=0 && row < m_pGeoItemModel->rowCount() )
	{
		QString name = m_pGeoItemModel->data(
					m_pGeoItemModel->index(row,0)
					).toString();

		refreshItemUI(name);
	}
}

void qtvplugin_geomarker::on_pushButton_QTV_getPolygon_clicked()
{
	if (!m_pVi)	return;
	QString strGridName = "grid";/*QString("grid%1").arg(m_nInstance);*/
	layer_interface * pif =  m_pVi->layer(strGridName);
	if (pif)
	{
		QMap<QString, QVariant> inPara, outPara;
		inPara["function"] = "get_polygon";
		outPara = pif->call_func(inPara);
		QString strPlainText = "";
		if (outPara.contains("size"))
		{
			int nsz = outPara["size"].toInt();
			for (int i=0;i<nsz;++i)
			{
				QString latkey = QString("lat%1").arg(i);
				QString lonkey = QString("lon%1").arg(i);
				strPlainText += QString("%1,%2;\n").arg(outPara[latkey].toDouble(),0,'f',7).arg(outPara[lonkey].toDouble(),0,'f',7);
			}
		}
		ui->plainTextEdit_QTV_corners->setPlainText(strPlainText);
	}
}
void qtvplugin_geomarker::refreshItemUI(QString markname)
{
	QString name = markname;
	//Fill in the pages
	QTVP_GEOMARKER::geoItemBase * item = m_pScene->geoitem_by_name(name);
	if (item)
	{
		ui->lineEdit_QTV_currentID->setText(item->item_name());
		//Update Pen and brush
		QPen pen;
		QBrush brush;
		switch (item->item_type())
		{
		case QTVP_GEOMARKER::ITEAMTYPE_RECT_POINT:
		{
			QTVP_GEOMARKER::geoGraphicsRectItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsRectItem *>(item);
			if (!pitem)
				break;
			pen = pitem->pen();
			brush = pitem->brush();
			ui->lineEdit_QTV_point_lat->setText(QString("%1").arg(pitem->lat(),0,'f',7));
			ui->lineEdit_QTV_point_lon->setText(QString("%1").arg(pitem->lon(),0,'f',7));
			ui->radioButton_QTV_PointRect->setChecked(true);
			ui->spinBox_QTV_point_width->setValue(pitem->width());
			ui->spinBox_QTV_point_height->setValue(pitem->height());
			ui->radioButton_QTV_tool_point->setChecked(true);
		}
			break;
		case QTVP_GEOMARKER::ITEAMTYPE_ELLIPSE_POINT:
		{
			QTVP_GEOMARKER::geoGraphicsEllipseItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsEllipseItem *>(item);
			if (!pitem)
				break;
			pen = pitem->pen();
			brush = pitem->brush();
			ui->lineEdit_QTV_point_lat->setText(QString("%1").arg(pitem->lat(),0,'f',7));
			ui->lineEdit_QTV_point_lon->setText(QString("%1").arg(pitem->lon(),0,'f',7));
			ui->radioButton_QTV_PointRound->setChecked(true);
			ui->spinBox_QTV_point_width->setValue(pitem->width());
			ui->spinBox_QTV_point_height->setValue(pitem->height());
			ui->radioButton_QTV_tool_point->setChecked(true);
		}
			break;
		case QTVP_GEOMARKER::ITEAMTYPE_LINE:
		{
			QTVP_GEOMARKER::geoGraphicsLineItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsLineItem *>(item);
			if (!pitem)
				break;
			pen = pitem->pen();
			ui->lineEdit_QTV_lineLat1->setText(QString("%1").arg(pitem->lat1(),0,'f',7));
			ui->lineEdit_QTV_lineLat2->setText(QString("%1").arg(pitem->lat2(),0,'f',7));
			ui->lineEdit_QTV_lineLon1->setText(QString("%1").arg(pitem->lon1(),0,'f',7));
			ui->lineEdit_QTV_lineLon2->setText(QString("%1").arg(pitem->lon2(),0,'f',7));
			ui->radioButton_QTV_tool_line->setChecked(true);
		}
			break;
		case QTVP_GEOMARKER::ITEAMTYPE_POLYGON:
		{
			QTVP_GEOMARKER::geoGraphicsPolygonItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsPolygonItem *>(item);
			if (!pitem)
				break;
			pen = pitem->pen();
			brush = pitem->brush();
			QPolygonF pol = pitem->llas();
			QString strPlainText;
			foreach (QPointF p, pol)
				strPlainText += QString("%1,%2;\n").arg(p.y(),0,'f',7).arg(p.x(),0,'f',7);
			ui->plainTextEdit_QTV_corners->setPlainText(strPlainText);
			ui->radioButton_QTV_tool_polygon->setChecked(true);
			ui->checkBox_QTV_multiline->setChecked(false);
		}
			break;
		case QTVP_GEOMARKER::ITEAMTYPE_PIXMAP:
		{
			QTVP_GEOMARKER::geoGraphicsPixmapItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsPixmapItem *>(item);
			if (!pitem)
				break;
			ui->lineEdit_QTV_icon_lat->setText(QString("%1").arg(pitem->lat(),0,'f',7));
			ui->lineEdit_QTV_icon_lon->setText(QString("%1").arg(pitem->lon(),0,'f',7));
			ui->radioButton_QTV_tool_icons->setChecked(true);
			ui->lineEdit_QTV_icon_rotate->setText(QString("%1").arg(pitem->rotation()));
			ui->lineEdit_QTV_icon_scale->setText(QString("%1").arg(pitem->scale()));
			QString nameicon = pitem->icon()->name;
			ui->comboBox_QTV_icons->setCurrentText(nameicon);
		}
			break;
		case QTVP_GEOMARKER::ITEAMTYPE_MULTILINE:
		{
			QTVP_GEOMARKER::geoGraphicsMultilineItem * pitem = dynamic_cast<QTVP_GEOMARKER::geoGraphicsMultilineItem *>(item);
			if (!pitem)
				break;
			pen = pitem->pen();
			brush = pitem->brush();
			QPolygonF pol = pitem->llas();
			QString strPlainText;
			foreach (QPointF p, pol)
				strPlainText += QString("%1,%2;\n").arg(p.y(),0,'f',7).arg(p.x(),0,'f',7);
			ui->plainTextEdit_QTV_corners->setPlainText(strPlainText);
			ui->radioButton_QTV_tool_polygon->setChecked(true);
			ui->checkBox_QTV_multiline->setChecked(true);
		}
			break;

		default:
			break;

		}//end switch

		QColor colorPen = pen.color();
		ui->lineEdit_QTV_PenColor->setText(color2string(colorPen));
		QColor colorFill = brush.color();
		ui->lineEdit_QTV_FillColor->setText(color2string(colorFill));
		int penwidth = pen.width();
		ui->spinBox_QTV_penWidth->setValue(penwidth);
		Qt::PenStyle st = pen.style();
		int nst = int(st);
		if (nst >=0 && nst < ui->comboBox_QTV_linePad->count())
			ui->comboBox_QTV_linePad->setCurrentIndex((int)st);
		Qt::BrushStyle bs = brush.style();
		int nbs = int(bs);
		if (nbs >=0 && nbs < ui->comboBox_QTV_fillPad->count())
			ui->comboBox_QTV_fillPad->setCurrentIndex((int)nbs);

		QColor colorText = item->labelColor();
		ui->lineEdit_QTV_TextColor->setText(color2string(colorText));

		int fsize = item->labelFont().pointSize();
		ui->spinBox_QTV_fontSize->setValue(fsize);

		int weight = item->labelFont().weight();
		ui->spinBox_QTV_textWeight->setValue(weight);

		refreshProps(item);
	}//end if item

}
void qtvplugin_geomarker::refreshIconModel()
{
	m_pIconsModel->clear();
	foreach (QString key, m_map_icons.keys())
	{
		QStandardItem * item =  new QStandardItem(key);
		QIcon icon(m_map_icons[key].icon);
		item->setIcon(icon);
		m_pIconsModel->appendRow(item);
	}
}

void qtvplugin_geomarker::refreshProps(QTVP_GEOMARKER::geoItemBase * itm)
{
	m_pGeoPropModel->removeRows(0,m_pGeoPropModel->rowCount());
	int ct = itm->prop_counts();
	QStringList lstNames = itm->prop_names();
	QVariantList lstValues = itm->prop_values();
	for (int i=0;i<ct;++i)
	{
		m_pGeoPropModel->appendRow(new QStandardItem(lstNames.first()));
		m_pGeoPropModel->setData(
					m_pGeoPropModel->index(i,1),
					lstValues.first());
		lstNames.pop_front();
		lstValues.pop_front();
	}
}
void qtvplugin_geomarker::on_pushButton_QTV_save_clicked()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("history/last_save_xml_dir","./").toString();
	QString newfm = QFileDialog::getSaveFileName(this,tr("save to xml"),strLastSaveImgDir,
								 "xml (*.xml);;All files(*.*)"
								 );
	if (newfm.size()>2)
	{
		if (true==xml_save(newfm))
		{
			settings.setValue("history/last_save_xml_dir",newfm);
			//QMessageBox::information(this,tr("succeed"),tr("Successfully saved XML file") + newfm);
		}
		else
			QMessageBox::warning(this,tr("failed"),tr("Save XML file") + newfm + tr(" Failed"));
	}
}

void qtvplugin_geomarker::on_pushButton_QTV_load_clicked()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("history/last_open_xml_dir","./").toString();
	QString newfm = QFileDialog::getOpenFileName(this,tr("load from xml"),strLastSaveImgDir,
								 "xml (*.xml);;All files(*.*)"
								 );
	if (newfm.size()>2)
	{
		if (true==xml_load(newfm))
		{
			settings.setValue("history/last_open_xml_dir",newfm);
			//QMessageBox::information(this,tr("succeed"),tr("Successfully load XML file") + newfm);
		}
		else
			QMessageBox::warning(this,tr("failed"),tr("Load XML file") + newfm + tr(" Failed"));
	}
	scheduleRefreshMarks();
	m_pVi->UpdateWindow();
}
void qtvplugin_geomarker::on_pushButton_QTV_import_icon_clicked()
{
	DialogSelectIcon dlg (ini_file()) ;
	if (dlg.exec()==QDialog::Accepted)
	{
		m_map_icons[dlg.m_icon.name] = dlg.m_icon;
	}
	refreshIconModel();
}
void qtvplugin_geomarker::on_pushButton_QTV_load_icons_clicked()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("history/last_save_icon_xml_dir","./").toString();
	QString newfm = QFileDialog::getOpenFileName(this,tr("load from xml"),strLastSaveImgDir,
								 "xml (*.xml);;All files(*.*)"
								 );
	if (newfm.size()>2)
	{
		if (true==xml_icon_load(newfm))
		{
			settings.setValue("history/last_load_icon_xml_dir",newfm);
			//QMessageBox::information(this,tr("succeed"),tr("Successfully load XML file") + newfm);
		}
		else
			QMessageBox::warning(this,tr("failed"),tr("Load XML file") + newfm + tr(" Failed"));
	}
	this->refreshIconModel();
}

void qtvplugin_geomarker::on_pushButton_QTV_save_icons_clicked()
{
	QSettings settings(ini_file(),QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("history/last_save_icon_xml_dir","./").toString();
	QString newfm = QFileDialog::getSaveFileName(this,tr("save to xml"),strLastSaveImgDir,
								 "xml (*.xml);;All files(*.*)"
								 );
	if (newfm.size()>2)
	{
		if (true==xml_icon_save(newfm))
		{
			settings.setValue("history/last_save_icon_xml_dir",newfm);
			//QMessageBox::information(this,tr("succeed"),tr("Successfully saved XML file") + newfm);
		}
		else
			QMessageBox::warning(this,tr("failed"),tr("Save XML file") + newfm + tr(" Failed"));
	}
}
void qtvplugin_geomarker::on_pushButton_QTV_collaps_all_clicked()
{
	QList< QTVP_GEOMARKER::geoItemBase *  > lst = m_pScene->geo_items();

	bool needupdate = false;
	foreach (QTVP_GEOMARKER::geoItemBase * base,lst)
	{
		bool bOldVis = base->props_visible();
		if (bOldVis)
			needupdate = true;
		base->show_props(false);
	}
	if (needupdate)
	{
		//scheduleRefreshMarks();
		scheduleUpdateMap();
	}
}
void qtvplugin_geomarker::on_pushButton_QTV_refresh_list_clicked()
{
	this->scheduleRefreshMarks();
}
void qtvplugin_geomarker::on_radioButton_QTV_display_clicked()
{
	if (!m_pVi)	return ;
	m_sel_ptStart_World = m_sel_ptEnd_World = QPointF();
	m_currentTools = qtvplugin_geomarker::TOOLS_DISPLAY_ONLY;
	layer_interface * pOSM =  m_pVi->layer("OSM");
	if (pOSM)
	{
		pOSM->set_active(true);
		m_pVi->adjust_layers(pOSM);
	}
	m_pVi->UpdateWindow();
	m_pVi->updateLayerGridView();
	ui->toolBox_QTV_marks->setCurrentIndex(0);

}

void qtvplugin_geomarker::on_radioButton_QTV_rect_selection_clicked()
{
	if (!m_pVi)	return ;
	m_currentTools = qtvplugin_geomarker::TOOLS_RECT_SELECTION;
	m_sel_ptStart_World = m_sel_ptEnd_World = QPointF();
	m_pVi->adjust_layers(this);
	m_pVi->UpdateWindow();
	m_pVi->updateLayerGridView();
	ui->toolBox_QTV_marks->setCurrentIndex(1);
}
void qtvplugin_geomarker::on_pushButton_QTV_sel_clear_clicked()
{
	clearSelection();
}

void qtvplugin_geomarker::on_pushButton_QTV_sel_delselected_clicked()
{
	foreach (QString namep,m_set_itemNameSelected)
	{
		QTVP_GEOMARKER::geoItemBase * b = m_pScene->geoitem_by_name(namep);
		if (b)
			m_pScene->removeItem(b,0);
	}
	clearSelection();
	scheduleRefreshMarks();
}
void qtvplugin_geomarker::on_pushButton_QTV_style_default_save_clicked()
{
	style_save();
}

void qtvplugin_geomarker::on_pushButton_QTV_style_default_load_clicked()
{
	style_load();
}
