
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui/QMainWindow>
#include <QFileDialog>
#include <QFileInfo>
#include <QDomDocument>
#include <QSettings>
#include <QTextStream>
#include <QMessageBox>

using namespace std;

#include "ui_main_window.h"

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    main_window(QWidget *parent = 0);
    ~main_window();
private:
    Ui::MainWindow ui;
	qtheight *qh;
};
#endif // MAIN_WINDOW_H
