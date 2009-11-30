
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
	void write_height(QString);
	void build_height(void);
	MagickWand *out_img;
	MagickWand *terrain_img;
	MagickWand *offset_img;
	QList<QString> nif_filenames;
	QList<QString> nif_name;

private slots:	    
	void open_filedialog_daoc(void);
	void open_filedialog_build(void);
	void run_build(void);
};
#endif // MAIN_WINDOW_H
