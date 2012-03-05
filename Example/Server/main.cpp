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

	return app.exec();
}
