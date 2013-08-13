#include "ServerThreaded.h"
#include <iostream>

ServerThreaded::ServerThreaded()
{
	int port = 1337;
	server = new QWsServer(this);
	if (! server->listen(QHostAddress::Any, port))
	{
		std::cout << QObject::tr("Error: Can't launch server").toStdString() << std::endl;
		std::cout << QObject::tr("QWsServer error : %1").arg(server->errorString()).toStdString() << std::endl;
	}
	else
	{
		std::cout << QObject::tr("Server is listening port %1").arg(port).toStdString() << std::endl;
	}
	QObject::connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

ServerThreaded::~ServerThreaded()
{
}

void ServerThreaded::processNewConnection()
{
	std::cout << QObject::tr("Client connected").toStdString() << std::endl;

	// Get the connecting socket
	QWsSocket* socket = server->nextPendingConnection();

	// Create a new thread and giving to him the socket
	SocketThread* thread = new SocketThread(socket);
	
	// connect for message broadcast
	QObject::connect(socket, SIGNAL(frameReceived(QString)), this, SIGNAL(broadcastMessage(QString)));
	QObject::connect(this, SIGNAL(broadcastMessage(QString)), thread, SLOT(sendMessage(QString)));

	// Starting the thread
	thread->start();
}
