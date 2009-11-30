#ifndef QTHEIGHT_H
#define QTHEIGHT_H

#include <zlib.h>
#include <string.h>
#include <stdlib.h>       

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QObject>
#include <QWidget>
#include <QString>
#include <QFile>
#include <QMap>
#include <QTextStream>
#include <wand/MagickWand.h>

#include "qtmpak.h"

using namespace std;

class qtheight : public QObject
{
    Q_OBJECT

public:
    qtheight(QWidget *parent = 0);
    ~qtheight();
	void build_height(void);
	bool extract(QString file);
	bool read_zone_structure(void);
private:
	void write_height(QString);
	bool extract_mpak(QString,QString,QString);
	void processData(int, char *, int,QString,QString);
	bool build_zone(QString);
	MagickWand *out_img;
	MagickWand *terrain_img;
	MagickWand *offset_img;
	QMap<QString,QString> zone_structure;
};

#endif // QTHEIGHT_H
