#ifndef QT_DEMPAK_H
#define QT_DEMPAK_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QFile>
#include <QDir>
#include <zlib.h>

class qt_dempak : public QObject
{
    Q_OBJECT

public:
	void upload(int, char *,int);
	bool init(QString);
	QString errorString;
	bool extract(QString,QString);
	qt_dempak();

private:
	QByteArray packetname_block;
	QByteArray filenames_block;
	int last_stage;
	QString packet;
	QStringList fileNames;
	QMap<QString,QByteArray> file_data;
};

#endif // QT_DEMPAK_H
