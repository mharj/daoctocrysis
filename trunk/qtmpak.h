#ifndef FILE_MPAK_H
#define FILE_MPAK_H

#include <QObject>
#include <QWidget>

using namespace std;

class file_mpak : public QObject
{
    Q_OBJECT

	public:
    	file_mpak(QWidget *parent = 0);
	    ~file_mpak();
		bool open(QString);


};











#endif // FILE_MPAK_H
