#include <QApplication>

#include "ServerExample.h"

#include "Log.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>

void myMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }

    Log::display(txt);
}


int main(int argc, char *argv[])
{
    qInstallMsgHandler(myMessageHandler);
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
