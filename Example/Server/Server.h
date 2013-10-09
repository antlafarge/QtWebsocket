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

#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"

class Server : public QObject
{
	Q_OBJECT

public:
	Server(int port = 80, QtWebsocket::Protocol protocol = QtWebsocket::Tcp);
	~Server();

public slots:
	void processNewConnection();
	void processMessage(QString message);
	void processPong(quint64 elapsedTime);
	void socketDisconnected();

private:
	QtWebsocket::QWsServer* server;
	QList<QtWebsocket::QWsSocket*> clients;
};

#endif // SERVER_H
