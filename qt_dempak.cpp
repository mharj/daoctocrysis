#include "qt_dempak.h"

qt_dempak::qt_dempak()
{
	last_stage=0;
}

bool qt_dempak::init(QString file)
{
	int inlen = 0;
	int stage = 0;
	char inbuf[16384],outbuf[16384];
	z_stream stream;
	FILE *fp;
	if (!(fp = fopen(file.toAscii(),"rb")))
	{
		errorString=QString("Error opening file %1").arg(file);
		return(false);
	}
	// read and check header
	fseek(fp, 0, SEEK_SET);
	fread (inbuf,1,4,fp);
	inbuf[4]='\0';
	if ( strcmp("MPAK",inbuf) )
	{
		errorString=QString("%1 not valid MPAK file").arg(file);
		return(false);	
	}
	// zstream start
	fseek(fp, 21, SEEK_SET);
	
	memset(&stream,0,sizeof(z_stream));

	inflateInit(&stream); 
	while (!feof(fp) || inlen > 0)
	{
		int rc=0;
		inlen += fread(inbuf + inlen, 1, sizeof(inbuf) - inlen, fp);
		stream.next_in = (unsigned char *)inbuf;
		stream.avail_in = inlen;
		stream.next_out = (unsigned char *)outbuf;
		stream.avail_out = sizeof(outbuf);
		rc = inflate(&stream, 0);
		if (rc != Z_STREAM_END && rc != Z_OK)
		{
			errorString=QString("inflate returned %d").arg(rc);
			return(false);
		}
        if ((char*)stream.next_out > outbuf)
			upload(stage, outbuf, (char*)stream.next_out - outbuf);

		if ((char*)stream.next_in > inbuf)
		{
			memmove(inbuf, stream.next_in, stream.avail_in);
			inlen = stream.avail_in;
		}
		if (rc == Z_STREAM_END)
		{
			++stage;
			inflateEnd(&stream);
			inflateInit(&stream);
		}
    }
	inflateEnd(&stream);
	fclose(fp);
	return(true);
}

bool qt_dempak::extract(QString path,QString filename)
{
	QDir check_path(path);
	if ( ! check_path.exists() )
		check_path.mkpath(path);

	if ( file_data.contains(filename) )
	{
		QFile file(QString("%1/%2").arg(path).arg(filename));
     	if (! file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
		{
			errorString=QString("Can't open file %1 to writing").arg(filename);
         	return(false);
		}
		file.write(file_data[filename]);
		file.close();
	}else
	{
		errorString=QString("Can't find data for file %1").arg(filename);
		return(false);
	}
	return(true);
}

void qt_dempak::upload (int stage,char *data,int len)
{
	// actions
	if ( stage != last_stage )
	{
		switch ( last_stage )
		{
			case 0:
				packet.append(QString(packetname_block.trimmed().toLower()));
				break;
			case 1:
				for (int i=0 ; i < filenames_block.size() ; i += 0x11c )
					fileNames.append(QString(filenames_block.mid(i,0x11c).trimmed().toLower()));
				break;
		}
	}

	if ( stage == 0 )
		packetname_block.append(data,len);	
	if ( stage == 1 )
		filenames_block.append(data,len);
	if ( stage > 1 && !fileNames.at( (stage-2) ).isNull() )
		file_data[ fileNames.at((stage-2)) ].append(data,len);

	last_stage=stage;		
}
