#include "dialogselecticon.h"
#include "ui_dialogselecticon.h"
#include <QSettings>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QFileInfo>
DialogSelectIcon::DialogSelectIcon(QString inifile,QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSelectIcon),
	iniFileName(inifile)
{
	ui->setupUi(this);
	QSettings settings(iniFileName,QSettings::IniFormat);
	ui->lineEdit_QTV_filename->setText(settings.value("IconSel/lineEdit_QTV_filename","./1.png").toString());
	ui->lineEdit_QTV_name->setText(settings.value("IconSel/lineEdit_QTV_name","1").toString());
	ui->lineEdit_QTV_centx->setText(settings.value("IconSel/lineEdit_QTV_centx","0").toString());
	ui->lineEdit_QTV_centy->setText(settings.value("IconSel/lineEdit_QTV_centy","0").toString());
}

DialogSelectIcon::~DialogSelectIcon()
{
	delete ui;
}

void DialogSelectIcon::on_pushButton_QTV_ok_clicked()
{
	QSettings settings(iniFileName,QSettings::IniFormat);
	settings.setValue("IconSel/lineEdit_QTV_filename",ui->lineEdit_QTV_filename->text());
	settings.setValue("IconSel/lineEdit_QTV_name",ui->lineEdit_QTV_name->text());
	settings.setValue("IconSel/lineEdit_QTV_centx",ui->lineEdit_QTV_centx->text());
	settings.setValue("IconSel/lineEdit_QTV_centy",ui->lineEdit_QTV_centy->text());

	m_icon.centerx = ui->lineEdit_QTV_centx->text().toInt();
	m_icon.centery = ui->lineEdit_QTV_centy->text().toInt();
	m_icon.filename = ui->lineEdit_QTV_filename->text();
	m_icon.name = ui->lineEdit_QTV_name->text();
	if (m_icon.icon.load(m_icon.filename))
		accept();
	else
	{
		QMessageBox::information(this,tr("Error open file"),m_icon.filename);
		reject();
	}
}

void DialogSelectIcon::on_pushButton_QTV_cancel_clicked()
{
	reject();
}

void DialogSelectIcon::on_toolButton_QTV_browser_clicked()
{
	QSettings settings(iniFileName,QSettings::IniFormat);
	QString strLastSaveImgDir = settings.value("IconSel/last_save_img_dir","./").toString();
	QString newfm = QFileDialog::getOpenFileName(this,tr("load from image"),strLastSaveImgDir,
								 "iamges (*.png;*.jpg;*.jpeg;*.ico;*.bmp;*.tiff);;All files(*.*)"
								 );
	if (newfm.size()>2)
	{
		QPixmap mp;
		if (true==mp.load(newfm))
		{
			ui->lineEdit_QTV_filename->setText(newfm);
			QFileInfo info(newfm);
			ui->lineEdit_QTV_name->setText(info.completeBaseName());
		}
		else
			QMessageBox::information(this,tr("Error open file"),newfm);
	}
}
