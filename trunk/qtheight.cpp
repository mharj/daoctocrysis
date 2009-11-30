#include "qtheight.h"
#include <QString>

qtheight::qtheight(QWidget *parent)
    : QObject(parent)
{	
	MagickWandGenesis();
	PixelWand *color;
	color=NewPixelWand();
	this->out_img=NewMagickWand();
	PixelSetBlack(color,(1.0f-1.0f));
	MagickNewImage(this->out_img,2048,2048,color);
	MagickSetImageColorspace(this->out_img,GRAYColorspace);
	MagickSetImageType(this->out_img,GrayscaleType);
	MagickSetImageFormat(this->out_img, "BMP" );
	MagickSetImageDepth(this->out_img, 16 );
}

qtheight::~qtheight()
{

}

void qtheight::build_height(void)
{
	QList<int> alb_zones;
//	alb_zones << 0 << 1 << 2 << 3 << 4 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 14 << 15;
	alb_zones << 0;
	this->read_zone_structure();	
	for (int i = 0; i < alb_zones.size(); ++i) 
	{
		QString mpk=QString("zone%L1/dat%L1.mpk").arg(alb_zones.at(i),3,10,QChar('0'));
		QString zonedir=QString("zone%L1").arg(alb_zones.at(i),3,10,QChar('0'));
		qDebug(zonedir.toAscii());
		this->extract_mpak(mpk,"terrain.pcx",zonedir);
		this->extract_mpak(mpk,"offset.pcx",zonedir);
		this->extract_mpak(mpk,"sector.dat",zonedir);
		this->build_zone(QString("%L1").arg(alb_zones.at(i),3,10,QChar('0')));
	}
	this->write_height("alb_height.bmp");

}

bool qtheight::read_zone_structure(void)
{
	QFile file("zone_structure.txt");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return(false);
	QTextStream zin(&file);	
	
	while (!zin.atEnd()) 
	{
		QString line = zin.readLine();
		QStringList attrs = line.split(":");
		this->zone_structure[attrs.at(0)]=QString("%1:%2").arg(attrs.at(1)).arg(attrs.at(2));
	}
	file.close();
	return(true);
}


bool qtheight::build_zone(QString zone_number)
{
	int status;
	int scalefactor=0;
	int offsetfactor=0;
	int tex_depth=0;
	int off_depth=0;
	float height=0;
	float *pixel_array;
	int *ipixel_array;
	int rx=0;
	int ry=0;
	pixel_array = new float[1];
	ipixel_array = new int[1];
	QStringList attrs;
	QString line;
	
	
	QStringList temp_sl;
	
	PixelWand *color;
	color = NewPixelWand();
	
	// get data from sector.dat
	QFile file(QString("zone%1/sector.dat").arg(zone_number) );
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return(false);
	QTextStream in(&file);
	while (!in.atEnd()) 
	{
		line = in.readLine();
		attrs = line.split("=");
		if ( attrs.at(0) == QString("scalefactor") )
			scalefactor=attrs.at(1).toInt();
		if ( attrs.at(0) == QString("offsetfactor") )
			offsetfactor=attrs.at(1).toInt();
	}
	file.close();
	
	if (! zone_structure.contains(zone_number) )
		return(false);

	attrs=zone_structure[zone_number].split(":");
	rx=attrs.at(0).toInt();
	ry=attrs.at(1).toInt();

	// terrain 
	this->terrain_img=NewMagickWand();
	status=MagickReadImage(this->terrain_img,QString("zone%1/terrain.pcx").arg(zone_number).toAscii());
	if ( status != 1 ) 
		return(false);
	
	// offset
	this->offset_img=NewMagickWand();
	status=MagickReadImage(this->offset_img,QString("zone%1/offset.pcx").arg(zone_number).toAscii());
	if ( status != 1 ) 
		return(false);		

	for (int y=0;y<256;y++)
	{	
		for (int x=0;x<256;x++)
		{
			MagickGetImagePixels(this->terrain_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			tex_depth=(int)(pixel_array[0]*256);
			MagickGetImagePixels(this->offset_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			off_depth=(int)(pixel_array[0]*256);
			height=(tex_depth*scalefactor) + (off_depth*offsetfactor);
			height*=2.5f;
			ipixel_array[0]=(int)height;
			MagickSetImagePixels(this->out_img,(rx*64)+x,(ry*64)+y,1,1,"I", ShortPixel,ipixel_array );
		}
	}
	return(true);
}

void qtheight::write_height(QString file)
{
	qDebug(QString("write:%1").arg(file).toAscii());
	MagickWriteImage(this->out_img,file.toAscii());
}

bool qtheight::extract_mpak(QString mpak,QString file,QString save_path)
{
	qt_dempak daoc;
	if (! daoc.init(mpak) )
	{
		qDebug(daoc.errorString.toAscii());
	}
	if (! daoc.extract(save_path,file) )
	{
		qDebug(daoc.errorString.toAscii());
	}
	return(true);
}
