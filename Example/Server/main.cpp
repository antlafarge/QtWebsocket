#include <QApplication>

#include "ServerExample.h"

#include "Log.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Log::display();

	ServerExample myServer;

	return app.exec();
}
