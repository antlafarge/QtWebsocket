/*
Copyright 2013 Antoine Lafarge qtwebsocket@gmail.com

This file is part of QtWebsocket.

QtWebsocket is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

QtWebsocket is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QtWebsocket.  If not, see <http://www.gnu.org/licenses/>.
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
	QObject::connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

Server::~Server()
{
}

void Server::processNewConnection()
{
	QtWebsocket::QWsSocket* clientSocket = server->nextPendingConnection();

	QObject::connect(clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)));
	QObject::connect(clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(clientSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	clients << clientSocket;

	std::cout << tr("Client connected").toStdString() << std::endl;
}

QString toReadableAscii(QString string)
{
	string.replace(QRegExp(QLatin1String("[ÀÁÂÃÄÅ]")), "ÀÁÂÃÄÅ");
	string.replace(QRegExp(QLatin1String("[àáâãäåª]")), "a");
	string.replace(QRegExp(QLatin1String("[ÈÉÊË£]")), "E");
	string.replace(QRegExp(QLatin1String("[èéêë]")), "e");
	string.replace(QRegExp(QLatin1String("[ÌÍÎÏ]")), "I");
	string.replace(QRegExp(QLatin1String("[ìíîï¡]")), "i");
	string.replace(QRegExp(QLatin1String("[ÒÓÔÕÖØ]")), "O");
	string.replace(QRegExp(QLatin1String("[òóôõöðø¤°º]")), "o");
	string.replace(QRegExp(QLatin1String("[ÙÚÛÜ]")), "U");
	string.replace(QRegExp(QLatin1String("[ùúûüµ]")), "u");
	string.replace(QRegExp(QLatin1String("[¥Ý]")), "Y");
	string.replace(QRegExp(QLatin1String("[ýÿ]")), "y");
	string.replace(QRegExp(QLatin1String("[Ç¢]")), "C");
	string.replace(QLatin1Char('ç'), "c");
	string.replace(QLatin1Char('©'), "(C)");
	string.replace(QLatin1Char('®'), "(R)");
	string.replace(QLatin1Char('«'), "<<");
	string.replace(QLatin1Char('»'), ">>");
	string.replace(QLatin1Char('¦'), "|");
	string.replace(QLatin1Char('§'), "S");
	string.replace(QLatin1Char('¨'), "\"");
	string.replace(QLatin1Char('¬'), "-");
	string.replace(QLatin1Char('-'), "-");
	string.replace(QLatin1Char('¯'), "-");
	string.replace(QLatin1Char('¹'), "^1");
	string.replace(QLatin1Char('²'), "^2");
	string.replace(QLatin1Char('³'), "^3");
	string.replace(QLatin1Char('±'), "+-");
	string.replace(QLatin1Char('¼'), "1/4");
	string.replace(QLatin1Char('½'), "1/2");
	string.replace(QLatin1Char('¾'), "3/4");
	string.replace(QLatin1Char('×'), "x");
	string.replace(QLatin1Char('÷'), "/");
	string.replace(QLatin1Char('´'), "`");
	string.replace(QLatin1Char('·'), ".");
	string.replace(QLatin1Char('¸'), ",");
	string.replace(QLatin1Char('¿'), "?");
	string.replace(QLatin1Char('¶'), "g");
	string.replace(QLatin1Char('Æ'), "AE");
	string.replace(QLatin1Char('æ'), "ae");
	string.replace(QLatin1Char('Ð'), "D");
	string.replace(QLatin1Char('Ñ'), "N");
	string.replace(QLatin1Char('ñ'), "n");
	string.replace(QLatin1Char('Þ'), "D");
	string.replace(QLatin1Char('þ'), "d");
	string.replace(QLatin1Char('ß'), "B");
	string.replace(QChar(0x20AC), "E");
	int i = string.size();
	while (i--)
	{
		QChar c = string.at(i);
		if (c < 32 || c > 126)
		{
			string[i] = ' ';
		}
	}
	return string;
}

void Server::processMessage(QString frame)
{
	QtWebsocket::QWsSocket* socket = qobject_cast<QtWebsocket::QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}
	std::cout << toReadableAscii(frame).toStdString() << std::endl;
	
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
