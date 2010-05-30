#include "main_window.h"


/*
 *	Read settings and make connections
 */ 
main_window::main_window(QWidget *parent)
    : QMainWindow(parent)  {
	ui.setupUi(this);
	// load settings
	QSettings settings("settings.ini", QSettings::IniFormat);
	ui.daocdir->setText( settings.value("path/daoc").toString() );
	ui.builddir->setText( settings.value("path/build").toString() );

	// init global image
	MagickWandGenesis();

	connect(ui.daocdir_button,SIGNAL( clicked() ),this,SLOT( open_filedialog_daoc() ) );
	connect(ui.builddir_button,SIGNAL( clicked() ),this,SLOT( open_filedialog_build() ) );
	connect(ui.startbutton,SIGNAL( clicked() ),this,SLOT( run_build() ) );
	
	QStringList tlist;
	tlist << "nif" << "cgf" << "count";
	ui.table_list->setRowCount(0);
	ui.table_list->setColumnCount(3);
	ui.table_list->setHorizontalHeaderLabels(tlist);
	ui.table_list->setSortingEnabled(true);
}

/*
 *	Write settings when closing
 */ 
main_window::~main_window() {
	QSettings settings("settings.ini", QSettings::IniFormat);
	settings.setValue("path/daoc", ui.daocdir->text() );
	settings.setValue("path/build", ui.builddir->text() );
}

/*
 * setup zone directory
 */ 
void main_window::open_filedialog_daoc() {
	QString Directory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	ui.daocdir->setText(Directory);
}

/*
 * setup build directory
 */ 
void main_window::open_filedialog_build() {
	QString Directory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	ui.builddir->setText(Directory);
}

void main_window::run_build() {
	QString build_area=QString(ui.map_select->currentText().toLower());
	
	domDocument.clear();
	Vegetation = domDocument.createElement("Vegetation");
	domDocument.appendChild(Vegetation);

	build_height(build_area);
	write_height(QString("%1/%2.bmp").arg(ui.builddir->text()).arg(build_area) );	
}

void main_window::read_tree_conversion(QString fileName)
{
	QString errorStr;
	int errorLine;
	int errorColumn;
	QString xml_data;
	QDomDocument domDocument;

	QString group_name;
	QString nif_key_name;
	QString cgf_name;
	QString snow;
	QString scale;
	
	// read xml
	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error",QString("Error reading file %1").arg(fileName), QMessageBox::Ok , QMessageBox::Ok);
		return;
	}
	
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		xml_data.append(line);
	}
	file.close();
	// DOM
	if (!domDocument.setContent(xml_data, true, &errorStr, &errorLine, &errorColumn)) {
		QMessageBox::critical(this, "Error",QString("Error reading XML data"), QMessageBox::Ok , QMessageBox::Ok);
	} else {
		QDomElement root = domDocument.documentElement();
		QDomNodeList entries=root.elementsByTagName("entry");
		for(int i=0;i<entries.count();i++) {
			QDomElement e = entries.item(i).toElement();
			QDomElement cell = e.elementsByTagName("cell").item(0).toElement();
			QString value = e.elementsByTagName("content").item(0).toElement().text();
			if (! cell.isNull() ) {
				int row=cell.attribute("row").toInt();
				int col=cell.attribute("col").toInt();
				if ( col == 1 && row > 1 )
					group_name=value;
				if ( col == 2 && row > 1 )
					nif_key_name=value.toLower();
				if ( col == 3 && row > 1 )
					cgf_name=value;
				if ( col == 4 && row > 1 )
					scale=value;
				if ( col == 5 && row > 1 ) {
					snow=value;
					nif_group[nif_key_name]=group_name;
					nif_cgf[nif_key_name]=cgf_name;
					nif_cgf_scale[nif_key_name]=scale.toDouble();
					if ( snow.toLower() == "false" )
						nif_snow[nif_key_name]=false;
					else
						nif_snow[nif_key_name]=true;
				}
			}
		}
	}
}

void main_window::build_height(QString subname) {
	qt_dempak zones_mpk;
	QList<int> conv_zones;
	
	region_offset_x.clear();
	region_offset_y.clear();
	scalefactor.clear();
	offsetfactor.clear();
	fixture_element.clear();
	nif_filenames.clear();
	nif_name.clear();
	
	// load conversion xml
	read_tree_conversion("veg.xml");	
	if ( subname == "albion" )
		conv_zones <<   0<<  1<<  2<<  3<<  4<<  6<<  7<<  8<<  9<< 10<< 11<< 12<< 14<< 15;
	if ( subname == "midgard" )
		conv_zones << 100<<101<<102<<103<<104<<105<<106<<107<<108<<116<<111<<112<<113<<115;
	if ( subname == "hibernia" )
		conv_zones << 200<<201<<202<<203<<204<<205<<206<<207<<208<<216<<210<<211<<212<<214;
	if ( subname == "thidranki" )
		conv_zones << 250;
	if ( subname == "new frontier" )
		conv_zones << 163<<164<<167<<168<<169<<170<<171<<172<<173<<174<<175<<176<<177<<178;
	if ( subname == "toa" )
		conv_zones <<  70<< 73<< 74<< 75<< 76<< 77<< 81<< 82<< 84<< 85<< 86<< 87;

	ui.logview->append(QString("extracting zone structure"));
	ui.logview->repaint();
	if ( ! zones_mpk.init( QString("%1/zones/zones.mpk").arg(ui.daocdir->text()) ) ) {
		QMessageBox::critical( 0, tr("Error"), zones_mpk.errorString );
		return;
	}
	
	if ( ! zones_mpk.extract(ui.builddir->text(),"zones.dat") ) {
		QMessageBox::critical( 0, tr("Error"), zones_mpk.errorString );
		return;
	}
	
	for (int i = 0; i < conv_zones.size(); ++i) {
		qt_dempak dem_files;
	
		QString id=QString("%L1").arg(conv_zones.at(i),3,10,QChar('0'));
		
		ui.logview->append(QString("extracting zone%1").arg(id));
		ui.logview->repaint();
		// check if zones or frontier/zones
		QString main_zone = QString("%1/zones/zone%2/dat%2.mpk").arg(ui.daocdir->text()).arg(id);
		QString frontier_zone = QString("%1/frontiers/zones/zone%2/dat%2.mpk").arg(ui.daocdir->text()).arg(id);
		
		if  (! dem_files.init( main_zone ) ) {
			if ( !  dem_files.init( frontier_zone ) ) {
				QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
				return;
			}
		}

		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"terrain.pcx") ) {
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"offset.pcx") ) {
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"sector.dat") ) {
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		// model and location data
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"nifs.csv") ) {
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		if (! dem_files.extract( QString("%1/zone%2").arg( ui.builddir->text() ).arg( id ),"fixtures.csv") ) {
			QMessageBox::critical( 0, tr("Error"), dem_files.errorString );
			return;
		}
		
		read_zone_offsets(id);
	}

	// calc zone offset
	calc_total_zone();
	// make image
	PixelWand *color;
	color=NewPixelWand();
	this->out_img=NewMagickWand();
	PixelSetBlack(color,(1.0f-1.0f));	
	MagickNewImage(this->out_img,image_size,image_size,color);
	MagickSetImageColorspace(this->out_img,GRAYColorspace);
	MagickSetImageType(this->out_img,GrayscaleType);
	MagickSetImageFormat(this->out_img, "BMP" );
	MagickSetImageDepth(this->out_img, 16 );
	
	for (int i = 0; i < conv_zones.size(); ++i) {
		QString id=QString("%L1").arg(conv_zones.at(i),3,10,QChar('0'));
		load_nif_list(id);
		build_zone(id);
		add_nifs_to_xml(id);
	}
	save_xml( QString("%1/%2.veg").arg( ui.builddir->text() ).arg(subname) );
	
	// build list
	QList<QString> keys = nif_counter.keys();
	ui.table_list->setRowCount(keys.length());
	for (int i=0; i < keys.length() ; i++ ) {
		QString nif_name_id=keys.at(i);
		QTableWidgetItem *idlabel = new QTableWidgetItem(tr("%1").arg(nif_name_id));
		QTableWidgetItem *idcount = new QTableWidgetItem();
		idcount->setData(Qt::DisplayRole, QVariant((int)nif_counter[nif_name_id]));
		QTableWidgetItem *niflabel = new QTableWidgetItem(tr("%1").arg(nif_cgf[nif_name_id]));
		ui.table_list->setItem(i, 0,idlabel );
		ui.table_list->setItem(i, 1,niflabel );
		ui.table_list->setItem(i, 2,idcount );
		
	}
	ui.table_list->resizeColumnsToContents();
}

void main_window::calc_total_zone (void) {
	int high_x=0;
	int high_y=0;
	low_x=-1;
	low_y=-1;	
	ui.logview->append(QString("scanning region offsets"));
	ui.logview->repaint();
	QList<int> keys = scalefactor.keys();
	for (int i = 0; i < keys.size(); ++i) {
		if ( low_x == -1 || low_x > region_offset_x[keys[i]] )
			low_x=region_offset_x[keys[i]];
		if ( low_y == -1 || low_y > region_offset_y[keys[i]] )
			low_y=region_offset_y[keys[i]];
		
		if ( high_x < region_offset_x[keys[i]] )
			high_x=region_offset_x[keys[i]];
		if ( high_y < region_offset_y[keys[i]] )
			high_y=region_offset_y[keys[i]];	
	}
	low_x*=32;
	low_y*=32;
	high_x*=32;
	high_y*=32;

	int comb_x=high_x-low_x;
	int comb_y=high_y-low_y;
	image_size=256;
	for (int i=1;i<10;i++) {	
		if ( comb_x < image_size && comb_y < image_size )
			break;
		image_size+=image_size;
	}
}

void main_window::load_nif_list (QString id) {
	QFile file(QString("%1/zone%2/nifs.csv").arg( ui.builddir->text() ).arg( id ));
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!file.atEnd()) {
		QString line(file.readLine());
		QStringList list = line.split(",");
		bool ok;
		int nif_id = list.at(0).toInt(&ok, 10 );
		if ( ok ) { // nif_id is numeric
			nif_filenames[nif_id]=list.at(2).toLower();
			nif_name[nif_id]=list.at(1);
		}
	}
	file.close();
}

void main_window::add_nifs_to_xml (QString id ) {
	QFile file(QString("%1/zone%2/fixtures.csv").arg( ui.builddir->text() ).arg( id ));
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!file.atEnd()) {
		QString line(file.readLine());
		QStringList list = line.split(",");
		bool ok;
		int nif_id = list.at(1).toInt(&ok, 10 );
		QString nif_name_id=nif_filenames.value(nif_id,QString("NULL"));
		if ( ok  ) {
			if ( ! nif_group[nif_name_id].isNull() ) {
				if ( fixture_element[nif_name_id].isNull() ) {
					VegetationObject = domDocument.createElement("VegetationObject");
					VegetationObject.setAttribute("FileName",nif_cgf[nif_name_id] );
//					VegetationObject.setAttribute("Category",QString("%1").arg( nif_group[nif_name_id] ) );
					VegetationObject.setAttribute("Category",QString("%1").arg( nif_name[nif_id] ) );
					VegetationObject.setAttribute("CastShadow","1");
					VegetationObject.setAttribute("RecvShadow","1");
					if ( nif_snow[nif_name_id] )
						VegetationObject.setAttribute("Frozen","1");
					else
						VegetationObject.setAttribute("Frozen","0");
				
					VegetationObject.setAttribute("nif_id",QString("%1").arg( nif_id ) );
					VegetationObject.setAttribute("nif_file",QString("%1").arg( nif_filenames.value(nif_id,QString("NULL")) ) );
					Vegetation.appendChild(VegetationObject);
					fixture_element[nif_name_id] = domDocument.createElement("Instances");
					VegetationObject.appendChild(fixture_element[nif_name_id]);
				}
				QDomElement Instance = domDocument.createElement("Instance");
//				ID,NIF #,Textual Name,X,Y,Z,A,Scale
				double x = (list.at(3).toDouble()/65536*1024)+(((region_offset_x[id.toInt()]*32)-low_x)*4);
				double y = (list.at(4).toDouble()/65536*1024)+(((region_offset_y[id.toInt()]*32)-low_y)*4);
				double z = list.at(5).toDouble()/64;
				double s = (list.at(7).toDouble()/100)/nif_cgf_scale[nif_name_id];
				Instance.setAttribute("Pos", QString("%1,%2,%3").arg(y,0,'f',3).arg(x,0,'f',3).arg(z,0,'f',3) );
				Instance.setAttribute("Scale",QString("%1").arg(s,0,'f',3) );
				fixture_element[nif_name_id].appendChild(Instance);
			}
			nif_counter[nif_name_id]++;			
		}
	}
	file.close();
}

void main_window::save_xml(QString file) {
	QFile nfile(file);			
	if (!nfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error",QString("Error open (write) file"), QMessageBox::Ok , QMessageBox::Ok);
		return;
	}
	nfile.write(domDocument.toByteArray());
	nfile.close();
	ui.logview->append(QString("saved vegetary layer %1").arg(file));
	ui.logview->repaint();
}

void main_window::read_zone_offsets(QString id) {
	// defined structure files
	QString zones_dat=QString("%1/zones.dat").arg(ui.builddir->text());
	QString onezone_dat=QString("%1/zone%2/sector.dat").arg(ui.builddir->text()).arg(id);	
	QSettings gsettings(zones_dat, QSettings::IniFormat);
	QSettings lsettings(onezone_dat, QSettings::IniFormat);
	
	// get structure data from files
	scalefactor[id.toInt()]=lsettings.value(QString("terrain/scalefactor"),0).toInt();
	offsetfactor[id.toInt()]=lsettings.value(QString("terrain/offsetfactor"),0).toInt();
	region_offset_x[id.toInt()]=gsettings.value(QString("zone%1/region_offset_x").arg(id),0).toInt();
	region_offset_y[id.toInt()]=gsettings.value(QString("zone%1/region_offset_y").arg(id),0).toInt();
}

void main_window::build_zone(QString id) {
	int status;	
	int tex_depth=0;
	int off_depth=0;
	float *pixel_array;
	int *ipixel_array;
	int height=0;
	pixel_array = new float[1];
	ipixel_array = new int[256];
	ui.logview->append(QString("building heightmap"));
	ui.logview->repaint();
	// terrain 
	this->terrain_img=NewMagickWand();
	status=MagickReadImage(this->terrain_img,QString("%1/zone%2/terrain.pcx").arg(ui.builddir->text()).arg(id).toAscii());
	if ( status != 1 ) {
		QMessageBox::critical( 0, tr("Error"), QString("No file found")); 
		return;
	}
	// offset
	this->offset_img=NewMagickWand();
	status=MagickReadImage(this->offset_img,QString("%1/zone%2/offset.pcx").arg(ui.builddir->text()).arg(id).toAscii());
	if ( status != 1 ) {
		QMessageBox::critical( 0, tr("Error"), QString("No file found")); 
		return;
	} 
	// read data from pcx files and combine to global map
	for (int y=0;y<256;y++) {	
		for (int x=0;x<256;x++) {
			MagickGetImagePixels(this->terrain_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			tex_depth=(int)(pixel_array[0]*256);
			MagickGetImagePixels(this->offset_img, x, y, 1, 1, "I", FloatPixel, pixel_array);
			off_depth=(int)(pixel_array[0]*256);
			height=(tex_depth*scalefactor[id.toInt()]) + (off_depth*offsetfactor[id.toInt()]);
			height*=4;
			if ( height >= 65536 )
				height=65535;
				
			ipixel_array[0]=height;
			MagickSetImagePixels(this->out_img,(region_offset_x[id.toInt()]*32)+x-low_x,(region_offset_y[id.toInt()]*32)+y-low_y,1,1,"I", ShortPixel,ipixel_array );
		}
	}
}

void main_window::write_height(QString file) {
	ui.logview->append(QString("write %1").arg(file));
	ui.logview->repaint();
	MagickWriteImage(this->out_img,file.toAscii());
}
