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

#include "SocketThread.h"
#include <QtCore>
#include <iostream>

SocketThread::SocketThread(QtWebsocket::QWsSocket* wsSocket) :
	socket(wsSocket)
{
	// Set this thread as parent of the socket
	// This will push the socket in the good thread when using moveToThread on the parent
	if (socket)
	{
		socket->setParent(this);
	}

	// Move this thread object in the thread himsleft
	// Thats necessary to exec the event loop in this thread
	moveToThread(this);
}

SocketThread::~SocketThread()
{
}

void SocketThread::run()
{
	std::cout << tr("connect done in thread : 0x%1").arg(QString::number((unsigned int)QThread::currentThreadId(), 16)).toStdString() << std::endl;

	// Connecting the socket signals here to exec the slots in the new thread
	QObject::connect(socket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)));
	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(socket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));
	QObject::connect(this, SIGNAL(finished()), this, SLOT(finished()), Qt::DirectConnection);

	// Launch the event loop to exec the slots
	exec();
}

void SocketThread::finished()
{
	this->moveToThread(QCoreApplication::instance()->thread());
	this->deleteLater();
}

void SocketThread::processMessage(QString message)
{
	// ANY PROCESS HERE IS DONE IN THE SOCKET THREAD !

	std::cout << tr("thread 0x%1 | %2").arg(QString::number((unsigned int)QThread::currentThreadId(), 16)).arg(message).toStdString() << std::endl;
}

void SocketThread::sendMessage(QString message)
{
	socket->write(message);
}

void SocketThread::processPong(quint64 elapsedTime)
{
	std::cout << tr("ping: %1 ms").arg(elapsedTime).toStdString() << std::endl;
}

void SocketThread::socketDisconnected()
{
	std::cout << tr("Client disconnected, thread finished").toStdString() << std::endl;

	// Prepare the socket to be deleted after last events processed
	socket->deleteLater();

	// finish the thread execution (that quit the event loop launched by exec)
	quit();
}
