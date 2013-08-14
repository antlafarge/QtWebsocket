#include "Server.h"
#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Server myServer;

	/*
	Webkit handshake tests
	QString accept1 = QWsServer::computeAcceptV0(QString("18x 6]8vM;54 *(5:  {   U1]8  z [  8"), QString("1_ tx7X d  <  nw  334J702) 7]o}` 0"), QString("Tm[K T2u"));
	QString accept2 = QWsServer::computeAcceptV0(QString("3e6b263  4 17 80"), QString("17  9 G`ZD9   2 2b 7X 3 /r90"), QString("WjN}|M(6"));
	QString accept3 = QWsServer::computeAcceptV0(QString("4 @1  46546xW%0l 1 5"), QString("12998 5 Y3 1  .P00"), QString("^n:ds[4U"));

	std::cout << QString(accept1.toLatin1().toHex()).toStdString() << std::endl << accept1.toStdString() << std::endl << std::endl;
	std::cout << QString(accept2.toLatin1().toHex()).toStdString() << std::endl << accept2.toStdString() << std::endl << std::endl;
	std::cout << QString(accept3.toLatin1().toHex()).toStdString() << std::endl << accept3.toStdString() << std::endl << std::endl;
	*/

	return app.exec();
}
