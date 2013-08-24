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

#ifndef QWSSERVER_H
#define QWSSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QSsl>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QNetworkProxy>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQueue>
#include <QFile>

#include "QWsSocket.h"

#include <iostream>

// SslServer over TcpServer for encrypted websocket connection
class SslServer : public QTcpServer
{
	Q_OBJECT

public:
	SslServer(QObject* parent = NULL) :
		QTcpServer(parent)
	{
		QObject::connect(this, SIGNAL(newConnection()), this, SLOT(test()));
	}
	virtual ~SslServer()
	{
	}

public slots:
	void displaySslErrors(const QList<QSslError>& errors)
	{
		for (int i=0, sz=errors.size(); i<sz; i++)
		{
			std::cout << errors.at(i).errorString().toStdString() << std::endl;
		}
	}

	void sslSocketEncrypted()
	{
		std::cout << "serverSocket ready (encryption OK)" << std::endl;
		QSslSocket* serverSocket = qobject_cast<QSslSocket*>(sender());
		emit newSslConnection(serverSocket);
	}

	void test()
	{
		std::cout << "tcp socket connected" << std::endl;
	}

signals:
	void newSslConnection(QSslSocket* serverSocket);

protected:
	virtual void SslServer::incomingConnection(qintptr socketDescriptor)
	{
		QSslSocket* serverSocket = new QSslSocket;
		QObject::connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(displaySslErrors(const QList<QSslError>&)));

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

			QObject::connect(serverSocket, SIGNAL(encrypted()), this, SLOT(sslSocketEncrypted()));
			serverSocket->startServerEncryption();
		}
		else
		{
			serverSocket->deleteLater();
		}
	}
};

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject* parent = 0, bool useSsl2 = false);
	// dtor
	virtual ~QWsServer();

	// ssl
	void setCertificate(const QSslCertificate &certificate, const QSslKey &key);

	// public functions
	void close();
	QString errorString();
	bool hasPendingConnections();
	bool isListening();
	bool listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
	int maxPendingConnections();
	virtual QWsSocket* nextPendingConnection();
	QNetworkProxy proxy();
	QHostAddress serverAddress();
	QAbstractSocket::SocketError serverError();
	quint16 serverPort();
	void setMaxPendingConnections(int numConnections);
	void setProxy(const QNetworkProxy & networkProxy);
	bool setSocketDescriptor(int socketDescriptor);
	int socketDescriptor();
	bool waitForNewConnection(int msec = 0, bool* timedOut = 0);

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection(QWsSocket* socket);
	virtual void incomingConnection(int socketDescriptor);

private slots:
	// private slots
	void newTcpConnection();
	void newSslConnection(QSslSocket* serverSocket);
	void closeTcpConnection();
	void dataReceived();
	void tcpSocketDisconnected();

private:
	// private attributes
	QTcpServer* tcpServer;
	QQueue<QWsSocket*> pendingConnections;
	QHash<const QTcpSocket*, QWsHandshake*> handshakeBuffer;

	bool useSsl;
	QSslKey sslKey;
	QSslCertificate sslCertificate;

public:
	// public static functions
	static QString composeOpeningHandshakeResponseV0(QByteArray accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "");
	static QString composeOpeningHandshakeResponseV4(QByteArray accept, QByteArray nonce, QString protocol = "", QString extensions = "");
	static QString composeOpeningHandshakeResponseV6(QByteArray accept, QString protocol = "", QString extensions = "");
	static QString composeBadRequestResponse(QList<EWebsocketVersion> versions = QList<EWebsocketVersion>());
};

#endif // QWSSERVER_H
