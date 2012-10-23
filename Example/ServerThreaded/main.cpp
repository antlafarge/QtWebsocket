#include <QApplication>
#include <QtDebug>

#include "ServerThreaded.h"

#include "Log.h"

void myMessageHandler( QtMsgType type, const char *msg )
{
	QString txt;

	switch (type)
	{
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
			break;
	}

	Log::display(txt);
}


int main(int argc, char *argv[])
{
	qInstallMsgHandler(myMessageHandler);

	QApplication app(argc, argv);

	Log::display();

    Log::display( "main thread : 0x" + QString::number((unsigned int)QThread::currentThreadId(), 16) );

	ServerThreaded myThreadedServer;

	return app.exec();
}
