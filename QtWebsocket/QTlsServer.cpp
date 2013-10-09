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

#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

namespace QtWebsocket
{

QTlsServer::QTlsServer(QObject* parent, Protocol allowedProtocols) :
	QTcpServer(parent),
	_allowedProtocols(allowedProtocols)
{
	QObject::connect(this, SIGNAL(newConnection()), this, SLOT(test()));
}

QTlsServer::~QTlsServer()
{
}

Protocol QTlsServer::allowedProtocols()
{
	return _allowedProtocols;
}

void QTlsServer::test()
{
	std::cout << "tcp socket connected, waiting for TLS handshake" << std::endl;
}

void QTlsServer::displayTlsErrors(const QList<QSslError>& errors)
{
	for (int i=0, sz=errors.size(); i<sz; i++)
	{
		std::cout << errors.at(i).errorString().toStdString() << std::endl;
	}
}

void QTlsServer::tlsSocketEncrypted()
{
	std::cout << "serverSocket ready (encryption OK)" << std::endl;
	QSslSocket* serverSocket = qobject_cast<QSslSocket*>(sender());
	emit newTlsConnection(serverSocket);
}
void QTlsServer::incomingConnection(qintptr socketDescriptor)
{
	QSslSocket* serverSocket = new QSslSocket;
	QObject::connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(displayTlsErrors(const QList<QSslError>&)));

	if (serverSocket->setSocketDescriptor(socketDescriptor))
	{
		QFile file("server-key.pem");
		if (!file.open(QIODevice::ReadOnly))
		{
			std::cout << "can't open key" << "server-key.pem";
			return;
		}
		QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray("qtwebsocket-server-key"));
		file.close();
		serverSocket->setPrivateKey(key);

		if (!serverSocket->addCaCertificates("ca.pem"))
		{
			std::cout << "open certificate ca error" << "ca.pem";
			return;
		}
		
		serverSocket->setLocalCertificate("server-crt.pem");
		serverSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
		//serverSocket->ignoreSslErrors();

		QObject::connect(serverSocket, SIGNAL(encrypted()), this, SLOT(tlsSocketEncrypted()));
		serverSocket->startServerEncryption();
	}
	else
	{
		serverSocket->deleteLater();
	}
}

} // namespace QtWebsocket
