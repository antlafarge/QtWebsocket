#include <QApplication>

#include "ServerExample.h"

#include "Log.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Log::display();

	ServerExample myServer;

/*
	// IGNORE THIS: That's just a test for webkit
	QString key1 = "^85u 6Q6Ln0 7566";
	QString key2 = "3 W 5B6YO2 Y 6 #7/4K4_{5";
	QByteArray key3BA = QByteArray::fromHex("AD7175A80C7100ED");
	QString key3( key3BA );
	QString accept = QWsServer::computeAcceptV1( key1, key2, key3 );
	QString acceptBA = accept.toAscii().toHex();
	Log::display( acceptBA );
*/
	return app.exec();
}
