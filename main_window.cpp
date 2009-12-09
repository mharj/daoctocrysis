#include "main_window.h"


/*
 *	Read settings and make connections
 */ 
main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	// load settings
	QSettings settings("settings.ini", QSettings::IniFormat);
	ui.daocdir->setText( settings.value("path/daoc").toString() );
	ui.builddir->setText( settings.value("path/build").toString() );

	// init global image
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
	
	connect(ui.daocdir_button,SIGNAL( clicked() ),this,SLOT( open_filedialog_daoc() ) );
	connect(ui.builddir_button,SIGNAL( clicked() ),this,SLOT( open_filedialog_build() ) );
	connect(ui.startbutton,SIGNAL( clicked() ),this,SLOT( run_build() ) );
	
	QDomDocument domDocument("");
	QDomElement Vegetation = domDocument.createElement("Vegetation");
	domDocument.appendChild(Vegetation);
	
}

/*
 *	Write settings when closing
 */ 
main_window::~main_window()
{
	QSettings settings("settings.ini", QSettings::IniFormat);
	settings.setValue("path/daoc", ui.daocdir->text() );
	settings.setValue("path/build", ui.builddir->text() );
}

/*
 * setup zone directory
 */ 
void main_window::open_filedialog_daoc()
{
	QString Directory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	ui.daocdir->setText(Directory);
}

/*
 * setup build directory
 */ 
void main_window::open_filedialog_build()
{
	QString Directory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	ui.builddir->setText(Directory);
}

void main_window::run_build()
{


	build_height();
	write_height(QString("%1/alb.bmp").arg(ui.builddir->text()));	
}

void main_window::build_height(void)
{
	qt_dempak zones_mpk;
	QList<int> alb_zones;
	
	alb_zones << 0 << 1 << 2 << 3 << 4 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 14 << 15;

	ui.logview->append(QString("extracting zone structure"));
	if ( ! zones_mpk.init( QString("%1/zones.mpk").arg(ui.daocdir->text()) ) )
	{
		QMessageBox::critical( 0, tr("Error"), zones_mpk.errorString );
		return;
	}
	
	if ( ! zones_mpk.extract(ui.builddir->text(),"zones.dat") )
	{
		QMessageBox::critical( 0, tr("Error"), zones_mpk.errorString );
		return;
	}
	for (int i = 0; i < alb_zones.size(); ++i) 
	{
		qt_dempak dem_files;
	
		QString id=QString("%L1").arg(alb_zones.at(i),3,10,QChar('0'));
		
		ui.logview->append(QString("adding zone%1").arg(id));
		
		if  (! dem_files.init( QString("%1/zone%2/dat%2.mpk").arg(ui.daocdir->text()).arg(id) ) )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}	
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"terrain.pcx") )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}		
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"offset.pcx") )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}		
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"sector.dat") )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		// model and location data
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"nifs.csv") )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"fixtures.csv") )
		{
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		load_nif_list(id);

		add_nifs_to_xml(id);
		
		build_zone(id);
	}
/*	for (int i=0;i < nif_name.count(); i++ )
	{
		if ( ! nif_name.at(i).isEmpty() )
			qDebug(nif_name.at(i).toAscii());
	}*/
}

void main_window::load_nif_list (QString id)
{
	QFile file(QString("%1/zone%2/nifs.csv").arg( ui.builddir->text() ).arg( id ));
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!file.atEnd()) 
	{
		QString line(file.readLine());
		QStringList list = line.split(",");
		bool ok;
		int nif_id = list.at(0).toInt(&ok, 10 );
		// nif is numeric
		if ( ok ) 
		{
		
			nif_filenames.insert(nif_id,list.at(2).toLower());
			nif_name.insert(nif_id,list.at(1));
		}

	}
	file.close();
}

void main_window::add_nifs_to_xml (QString id )
{
	qDebug("main_window::add_nifs_to_xml");
	QFile file(QString("%1/zone%2/fixtures.csv").arg( ui.builddir->text() ).arg( id ));
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!file.atEnd()) 
	{
		QString line(file.readLine());
		QStringList list = line.split(",");
		bool ok;
		int nif_id = list.at(1).toInt(&ok, 10 );
		// nif is numeric
		if ( ok ) 
		{
			qDebug("main_window::add_nifs_to_xml - loop");
//			ui.logview->append( list.at(2) );
			if ( fixture_element[nif_id].isNull() )
			{
				qDebug("main_window::add_nifs_to_xml - current null");
				//ui.logview->append( list.at(2) );
				VegetationObject = domDocument.createElement("VegetationObject");
				Vegetation.appendChild(VegetationObject);
				//qDebug("main_window::add_nifs_to_xml - current null 2");
//			QDomElement fixture_element[nif_id] = domDocument.createElement("Instances");
				fixture_element[nif_id] = domDocument.createElement("Instances");
				VegetationObject.appendChild(fixture_element[nif_id]);
//				fixture_element.insert(nif_id,Instances);
			}
//			qDebug("main_window::add_nifs_to_xml - done");
			QDomElement Instance = domDocument.createElement("Instance");
			Instance.setAttribute("Pos", "3200,3188,12.908286");
			Instance.setAttribute("Scale","0.2");
			Instance.setAttribute("Rotate","0.275637355817,0,0,-0.96126169593832");
			fixture_element[nif_id].appendChild(Instance);			
/*			
			QDomElement test = domDocument.documentElement();
			QDomElement SubMaterials = test.firstChildElement("SubMaterials");
	*/		
//			nif_filenames.insert(nif_id,list.at(2).toLower());
//			nif_name.insert(nif_id,list.at(1));
		}

	}
	file.close();
			
	// save new file			
	// korjaa tämä lopuksi <- <- <-
	QFile nfile("c:/temp/test.xml");			
	if (!nfile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error",QString("Error open (write) file"), QMessageBox::Ok , QMessageBox::Ok);
		return;
	}
	QTextStream nout(&nfile);
	nout <<	domDocument.toString();
	nfile.close();	
//	QMessageBox::critical(this, "Error",domDocument.toString(), QMessageBox::Ok , QMessageBox::Ok);
}

void main_window::build_zone(QString id)
{
	qDebug("main_window::build_zone");
	int status;	
	int tex_depth=0;
	int off_depth=0;
	float *pixel_array;
	int *ipixel_array;
	int height=0;
	pixel_array = new float[1];
	ipixel_array = new int[256];
	
	// defined structure files
	QString zones_dat=QString("%1/zones.dat").arg(ui.builddir->text());
	QString onezone_dat=QString("%1/zone%2/sector.dat").arg(ui.builddir->text()).arg(id);	
	QSettings gsettings(zones_dat, QSettings::IniFormat);
	QSettings lsettings(onezone_dat, QSettings::IniFormat);
	
	// get structure data from files
	int scalefactor=lsettings.value(QString("terrain/scalefactor"),0).toInt();
	int offsetfactor=lsettings.value(QString("terrain/offsetfactor"),0).toInt();
	int region_offset_x=gsettings.value(QString("zone%1/region_offset_x").arg(id),0).toInt();
	int region_offset_y=gsettings.value(QString("zone%1/region_offset_y").arg(id),0).toInt();

	// terrain 
	this->terrain_img=NewMagickWand();
	status=MagickReadImage(this->terrain_img,QString("%1/zone%2/terrain.pcx").arg(ui.builddir->text()).arg(id).toAscii());
	if ( status != 1 )
	{
		QMessageBox::critical( 0, tr("Error"), QString("No file found")); 
		return;
	}
	// offset
	this->offset_img=NewMagickWand();
	status=MagickReadImage(this->offset_img,QString("%1/zone%2/offset.pcx").arg(ui.builddir->text()).arg(id).toAscii());
	if ( status != 1 )
	{
		QMessageBox::critical( 0, tr("Error"), QString("No file found")); 
		return;
	} 
	// read data from pcx files and combine to global map
	for (int y=0;y<256;y++)
	{	
		for (int x=0;x<256;x++)
		{
			MagickGetImagePixels(this->terrain_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			tex_depth=(int)(pixel_array[0]*256);
			MagickGetImagePixels(this->offset_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			off_depth=(int)(pixel_array[0]*256);
			height=(tex_depth*scalefactor) + (off_depth*offsetfactor);
			height*=4;
			ipixel_array[0]=height;
			MagickSetImagePixels(this->out_img,(region_offset_x*32)+x-1024,(region_offset_y*32)+y-1024,1,1,"I", ShortPixel,ipixel_array );
		}
	}
}

void main_window::write_height(QString file)
{
	ui.logview->append(QString("write %1").arg(file));
//	qDebug(QString("write: %1").arg(file).toAscii());
	MagickWriteImage(this->out_img,file.toAscii());
}
