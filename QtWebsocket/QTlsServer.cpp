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

#include "QTlsServer.h"

#include <QDebug>

namespace QtWebsocket
{

QTlsServer::QTlsServer(const QSslConfiguration& sslConfiguration,
					   const QList<QSslCertificate> &caCertificates,
					   QObject* parent) :
	QTcpServer(parent),
	sslConfiguration(sslConfiguration),
	caCertificates(caCertificates)
{
}

QTlsServer::~QTlsServer()
{
}

void QTlsServer::displayTlsErrors(const QList<QSslError>& errors)
{
	for (int i=0, sz=errors.size(); i<sz; i++)
	{
		qDebug() << errors.at(i).errorString();
	}
}

void QTlsServer::tlsSocketEncrypted()
{
	QSslSocket* serverSocket = qobject_cast<QSslSocket*>(sender());
	emit newTlsConnection(serverSocket);
}

void QTlsServer::incomingConnection(qintptr socketDescriptor)
{
	QSslSocket* serverSocket = new QSslSocket;

	if (serverSocket->setSocketDescriptor(socketDescriptor))
	{
		QObject::connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(displayTlsErrors(const QList<QSslError>&)));

		serverSocket->setSslConfiguration(sslConfiguration);
		serverSocket->addCaCertificates(caCertificates);
		
		//serverSocket->ignoreSslErrors();

		QObject::connect(serverSocket, SIGNAL(encrypted()), this, SLOT(tlsSocketEncrypted()));
		serverSocket->startServerEncryption();
	}
	else
	{
		delete serverSocket;
	}
}

} // namespace QtWebsocket
