#include "Server.h"
#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Server myServer;

	return app.exec();
}
