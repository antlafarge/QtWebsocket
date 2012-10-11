#include <QtGui/QApplication>
#include "clientexample.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientExample w;
    w.show();
    
    return a.exec();
}
