
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui/QMainWindow>
#include <QFileDialog>
#include <QFileInfo>
#include <QDomDocument>
#include <QSettings>
#include <QTextStream>
#include <QMessageBox>
#include <QtGlobal>
#include <wand/MagickWand.h>
#include <math.h>

using namespace std;

#include "ui_main_window.h"
#include "qt_dempak.h"

class main_window : public QMainWindow
{
	Q_OBJECT

public:
	main_window(QWidget *parent = 0);
	~main_window();
private:
	Ui::MainWindow ui;
	void build_zone(QString);
	void load_nif_list(QString);
	void add_nifs_to_xml(QString);
	void save_xml(QString);
	void write_height(QString);
	void read_zone_offsets(QString);
	void calc_total_zone(void);
	void build_height(QString);
	void read_tree_conversion(QString);
	MagickWand *out_img;
	MagickWand *terrain_img;
	MagickWand *offset_img;
	QDomDocument domDocument;
	QDomElement root;
	QDomElement Vegetation;
	QDomElement VegetationObject;
	QMap<int,QString> nif_filenames;
	QMap<int,QString> nif_name;
	
	QMap<QString,QDomElement> fixture_element;
	QMap<int,int> scalefactor;
	QMap<int,int> offsetfactor;
	QMap<int,int> region_offset_x;
	QMap<int,int> region_offset_y;
	
	QMap<QString,QString> nif_group;
	QMap<QString,QString> nif_cgf;
	QMap<QString,double> nif_cgf_scale;
	QMap<QString,bool> nif_snow;
	
	
	
	int image_size;
	int low_x;
	int low_y;

private slots:	    
	void open_filedialog_daoc(void);
	void open_filedialog_build(void);
	void run_build(void);
};
#endif // MAIN_WINDOW_H
