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

#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>

#include "QWsSocket.h"

class SocketThread : public QThread
{
	Q_OBJECT

public:
	SocketThread(QtWebsocket::QWsSocket* wsSocket);
	~SocketThread();

	QtWebsocket::QWsSocket* socket;
	void run();

private slots:
	void processMessage(QString message);
	void sendMessage(QString message);
	void processPong(quint64 elapsedTime);
	void socketDisconnected();
	void finished();

signals:
	void messageReceived(QString frame);

private:
	
};

#endif // SOCKETTHREAD_H
