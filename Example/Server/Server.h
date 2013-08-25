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
	Server(int port = 80, bool useSsl = false);
	~Server();

public slots:
	void processNewConnection();
	void processMessage(QString message);
	void processPong(quint64 elapsedTime);
	void socketDisconnected();

private:
	QWsServer* server;
	QList<QWsSocket*> clients;
};

#endif // SERVER_H
