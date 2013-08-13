#include "ServerThreaded.h"
#include <QtCore>
#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::cout << QObject::tr("main thread : 0x%1").arg(QString::number((unsigned int)QThread::currentThreadId(), 16)).toStdString() << std::endl;

	ServerThreaded myThreadedServer;

	return app.exec();
}
