#include <QtGui/QApplication>
#include "Client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client w;
    w.show();

	int number = 1337;

    return a.exec();
}
