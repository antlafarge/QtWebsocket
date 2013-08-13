#include "Server.h"
#include <iostream>

Server::Server()
{
	int port = 1337;
	server = new QWsServer(this);
	if (! server->listen(QHostAddress::Any, port))
	{
		std::cout << tr("Error: Can't launch server").toStdString() << std::endl;
		std::cout << tr("QWsServer error : %1").arg(server->errorString()).toStdString() << std::endl;
	}
	else
	{
		std::cout << tr("Server is listening on port %1").arg(port).toStdString() << std::endl;
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

Server::~Server()
{
}

void Server::processNewConnection()
{
	QWsSocket* clientSocket = server->nextPendingConnection();

	connect(clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)));
	connect(clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	connect(clientSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	clients << clientSocket;

	std::cout << tr("Client connected").toStdString() << std::endl;
}

void Server::processMessage(QString frame)
{
	QWsSocket* socket = qobject_cast<QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}

	std::cout << frame.toStdString() << std::endl;
	
	QWsSocket* client;
	foreach (client, clients)
	{
		client->write(frame);
	}
}

void Server::processPong(quint64 elapsedTime)
{
	std::cout << tr("ping: %1 ms").arg(elapsedTime).toStdString() << std::endl;
}

void Server::socketDisconnected()
{
	QWsSocket* socket = qobject_cast<QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}

	clients.removeOne(socket);

	socket->deleteLater();

	std::cout << tr("Client disconnected").toStdString() << std::endl;
}
