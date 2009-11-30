#include "qtheight.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	qtheight *test;
	
	test = new qtheight();
	test->build_height();
	delete test;	
//    main_window w;
//    w.show();
//    return a.exec();

	return(0);
}
