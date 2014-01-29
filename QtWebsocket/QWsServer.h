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
#include "QTlsServer.h"

#include <iostream>

namespace QtWebsocket
{

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject* parent = 0, Protocol allowedProtocols = Tcp);
	// dtor
	virtual ~QWsServer();

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
	Protocol allowedProtocols();

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection(QWsSocket* socket);
	virtual void incomingConnection(int socketDescriptor);

private slots:
	// private slots
	void newTcpConnection();
	void newTlsConnection(QSslSocket* serverSocket);
	void closeTcpConnection();
	void dataReceived();
	void tcpSocketDisconnected();

private:
	// private attributes
	QTcpServer* tcpServer;
	QTlsServer tlsServer;
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

} // namespace QtWebsocket

#endif // QWSSERVER_H
