#include <QCoreApplication>

#include "TestServer.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	TestServer server;

	return app.exec();
}
