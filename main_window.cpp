#include "main_window.h"


/*
 *	Read settings and make connections
 */ 
main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	qh = new qtheight();
}

/*
 *	Write settings when closing
 */ 
main_window::~main_window()
{
	delete qh;
}

