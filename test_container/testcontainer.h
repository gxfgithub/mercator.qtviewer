#ifndef TESTCONTAINER_H
#define TESTCONTAINER_H

#include <QDialog>
#include <QStandardItemModel>
namespace Ui {
	class testcontainer;
}

class testcontainer : public QDialog
{
	Q_OBJECT
private:
	QStandardItemModel * m_pModel;
public:
	explicit testcontainer(QWidget *parent = 0);
	~testcontainer();
protected:
	void closeEvent(QCloseEvent *);
	void timerEvent(QTimerEvent *);
private:
	Ui::testcontainer *ui;
	int m_nAnTimer;
	void show_message(QString);
protected slots:
	void slot_message(QString);
	void on_pushButton_QTV_test_adds_clicked();
	void on_pushButton_QTV_test_cache_clicked();
	void on_pushButton_QTV_test_autodl_clicked();
	void on_pushButton_QTV_test_navigate_clicked();
	void on_pushButton_QTV_test_layers_clicked();
	void on_pushButton_QTV_test_layer_move_clicked();
	void on_pushButton_QTV_test_grid_enable_clicked();
	void on_pushButton_QTV_test_grid_getPolygon_clicked();
	void on_pushButton_QTV_test_mark_clicked();
	void on_pushButton_QTV_test_line_clicked();
	void on_pushButton_QTV_test_polygon_clicked();
	void on_pushButton_QTV_test_request_clicked();
	void on_pushButton_QTV_test_xml_clicked();
	void on_pushButton_QTV_test_resource_clicked();
	void on_pushButton_QTV_test_geo_displayMod_clicked();
	void on_pushButton_QTV_test_geo_selectionMod_clicked();
	void on_pushButton_QTV_test_geo_selected_marks_clicked();
	void on_pushButton_QTV_test_geo_clear_sel_clicked();
	void on_pushButton_QTV_test_geo_del_sel_clicked();
	void on_pushButton_QTV_default_style_clicked();
	void on_osmmap_map_event(QMap<QString, QVariant> p);
	void on_pushButton_QTV_test_10000_clicked();
};

#endif // TESTCONTAINER_H
