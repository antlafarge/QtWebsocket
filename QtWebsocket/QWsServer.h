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
#include <QNetworkProxy>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQueue>

#include "QWsSocket.h"

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject* parent = 0);
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

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection(QWsSocket* socket);
	virtual void incomingConnection(int socketDescriptor);

private slots:
	// private slots
	void newTcpConnection();
	void closeTcpConnection();
	void dataReceived();
	void disconnected();

private:
	// private attributes
	QTcpServer* tcpServer;
	QQueue<QWsSocket*> pendingConnections;
	QMap<const QTcpSocket*, QString*> headerBuffer;

public:
	// public static functions
	static QByteArray computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray thirdPart);
	static QByteArray computeAcceptV4(QByteArray key);
	static QByteArray generateNonce();
	static QString composeOpeningHandshakeResponseV0(QByteArray accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "");
	static QString composeOpeningHandshakeResponseV4(QByteArray accept, QByteArray nonce, QString protocol = "", QString extensions = "");
	static QString composeOpeningHandshakeResponseV6(QByteArray accept, QString protocol = "", QString extensions = "");
	static QString composeBadRequestResponse(QList<EWebsocketVersion> versions = QList<EWebsocketVersion>());

	// public static vars
	static QRegExp regExpHttpRequest;
	static QRegExp regExpHost;
	static QRegExp regExpKey;
	static QRegExp regExpKey1;
	static QRegExp regExpKey2;
	static QRegExp regExpVersion;
	static QRegExp regExpOrigin;
	static QRegExp regExpProtocol;
	static QRegExp regExpExtensions;
};

#endif // QWSSERVER_H
