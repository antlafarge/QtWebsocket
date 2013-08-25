/*
Copyright (C) 2013 Antoine Lafarge qtwebsocket@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Server.h"
#include <iostream>

Server::Server(int port, QtWebsocket::Protocol protocol)
{
	server = new QtWebsocket::QWsServer(this, protocol);
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
	QtWebsocket::QWsSocket* clientSocket = server->nextPendingConnection();

	connect(clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)));
	connect(clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	connect(clientSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	clients << clientSocket;

	std::cout << tr("Client connected").toStdString() << std::endl;
}

void Server::processMessage(QString frame)
{
	QtWebsocket::QWsSocket* socket = qobject_cast<QtWebsocket::QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}

	std::cout << frame.toStdString() << std::endl;
	
	QtWebsocket::QWsSocket* client;
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
	QtWebsocket::QWsSocket* socket = qobject_cast<QtWebsocket::QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}

	clients.removeOne(socket);

	socket->deleteLater();

	std::cout << tr("Client disconnected").toStdString() << std::endl;
}
