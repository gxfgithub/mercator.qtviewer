#include "layer_tiles_page.h"
#include "ui_layer_tiles_page.h"
#include "layer_tiles.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileDialog>
#include <QDebug>
#include <QCheckBox>
namespace QTVOSM{
	layer_tiles_page::layer_tiles_page(layer_tiles * layer,QWidget *parent) :
		QWidget(parent),
		m_pLayer(layer),
		ui(new Ui::layer_tiles_page)
	{
		ui->setupUi(this);
		//Get Cache Address
		QSettings settings(QCoreApplication::applicationFilePath()+".ini",QSettings::IniFormat);
		QString strServerURL = settings.value(QString("settings/ServerURL_%1").arg(layer->get_name()),"http://c.tile.openstreetmap.org/%1/%2/%3.png").toString();
		QString strLocalCache = settings.value(QString("settings/LocalCache_%1").arg(layer->get_name()), QCoreApplication::applicationDirPath() +"/OSMCache").toString();
		int nCacheExpireDays = settings.value(QString("settings/CacheExpireDays_%1").arg(layer->get_name()), 30).toInt();
		int nAutoDownload = settings.value(QString("settings/nAutoDownload_%1").arg(layer->get_name()), 0).toInt();

		ui->lineEdit_QTV_cacheFolder->setText(strLocalCache);
		ui->lineEdit_QTV_addressUrl->setText(strServerURL);
		ui->spinBox_QTV_cacheExpireDays->setValue(nCacheExpireDays);
		this->setWindowTitle(layer->get_name());
		//the pending tasks model
		m_pPendingTasksModel = new QStandardItemModel(this);
		ui->listView_QTV_messages->setModel(m_pPendingTasksModel);

		connect (layer, &layer_tiles::connected ,this->ui->checkBox_QTV_connect, &QCheckBox::setChecked);
		connect (layer, &layer_tiles::svrurlChanged ,this->ui->lineEdit_QTV_addressUrl, &QLineEdit::setText);
		connect (layer, &layer_tiles::cacheChanged ,this->ui->lineEdit_QTV_cacheFolder, &QLineEdit::setText);
		connect (layer, &layer_tiles::cacheExpireChanged ,this->ui->spinBox_QTV_cacheExpireDays, &QSpinBox::setValue);
		//ui->checkBox_QTV_connect->setChecked(layer->isConnected());
		if (nAutoDownload!=0)
			layer->connectToTilesServer(true);

	}
	void layer_tiles_page::reTransUI()
	{
		ui->retranslateUi(this);
	}

	layer_tiles_page::~layer_tiles_page()
	{
		delete ui;
	}
	void layer_tiles_page::on_toolButton_QTV_browser_clicked()
	{
		QSettings settings(QCoreApplication::applicationFilePath()+".ini",QSettings::IniFormat);
		QString strLocalCache = settings.value(QString("settings/LocalCache_%1").arg(m_pLayer->get_name()), QCoreApplication::applicationDirPath() +"/OSMCache").toString();
		QString strFolder = QFileDialog::getExistingDirectory(this,tr("Select the local cache"),strLocalCache,QFileDialog::ShowDirsOnly);
		if (strFolder.length())
			ui->lineEdit_QTV_cacheFolder->setText(strFolder);
	}

	void layer_tiles_page::on_pushButton_QTV_apply_clicked()
	{
		m_pLayer->setLocalCache(ui->lineEdit_QTV_cacheFolder->text());
		m_pLayer->setServerUrl(ui->lineEdit_QTV_addressUrl->text());
		m_pLayer->setCacheExpireDays(ui->spinBox_QTV_cacheExpireDays->value());
		m_pLayer->UpdateLayer();
	}

	void layer_tiles_page::on_checkBox_QTV_connect_clicked(bool ps)
	{
		m_pLayer->connectToTilesServer(ps);
	}
	void layer_tiles_page::slot_message(QString message)
	{
		m_pPendingTasksModel->appendRow(new QStandardItem(message));
		if (m_pPendingTasksModel->rowCount()>128)
			m_pPendingTasksModel->removeRows(0,m_pPendingTasksModel->rowCount()-128);
		ui->listView_QTV_messages->scrollToBottom();
	}
}
