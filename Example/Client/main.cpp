#include "Client.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Client w;
	w.show();

	return app.exec();
}
