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

class QTcpServer;
class QTcpSocket;

#include <QSslConfiguration>
#include <QNetworkProxy>
#include <QString>
#include <QQueue>
#include <QFile>

#include "WsEnums.h"

namespace QtWebsocket
{

class QTlsServer;
class QWsSocket;
class QWsHandshake;

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject* parent = 0, Protocol allowedProtocol = Tcp,
			  const QSslConfiguration& sslConfiguration = QSslConfiguration::defaultConfiguration(),
			  const QList<QSslCertificate>& caCertificates = QList<QSslCertificate>());
	// dtor
	virtual ~QWsServer();

	// public functions
	Protocol allowedProtocol();
	bool hasPendingConnections();
	bool isListening();
	bool listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
	void close();
	int maxPendingConnections();
	virtual QWsSocket* nextPendingConnection();
	QAbstractSocket::SocketError serverError();
	QString errorString();
	QNetworkProxy proxy();
	QHostAddress serverAddress();
	quint16 serverPort();
	void setMaxPendingConnections(int numConnections);
	void setProxy(const QNetworkProxy & networkProxy);
	bool waitForNewConnection(int msec = 0, bool* timedOut = 0);
	bool setSocketDescriptor(int socketDescriptor);
	int socketDescriptor();

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection(QWsSocket* socket);

private slots:
	// private slots
	void newTcpConnection();
	void newTlsConnection(QSslSocket* serverSocket);
	void closeTcpConnection();
	void dataReceived();
	void tcpSocketDisconnected();

private:
	// private attributes
	QTlsServer* tlsServer;
	QQueue<QWsSocket*> pendingConnections;
	QHash<const QTcpSocket*, QWsHandshake*> handshakeBuffer;

public:
	// public static functions
	static QString composeOpeningHandshakeResponseV0(QByteArray accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "");
	static QString composeOpeningHandshakeResponseV4(QByteArray accept, QByteArray nonce, QString protocol = "", QString extensions = "");
	static QString composeOpeningHandshakeResponseV6(QByteArray accept, QString protocol = "", QString extensions = "");
	static QString composeBadRequestResponse(QList<EWebsocketVersion> versions = QList<EWebsocketVersion>());
};

} // namespace QtWebsocket

#endif // QWSSERVER_H
