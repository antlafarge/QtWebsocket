#include <QApplication>

#include "ServerExample.h"

#include "Log.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Log::display();

	ServerExample myServer;

	return a.exec();
}
