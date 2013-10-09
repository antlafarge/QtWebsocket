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

#include "QWsServer.h"

#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

namespace QtWebsocket
{

QWsServer::QWsServer(QObject* parent, Protocol allowedProtocols)
	: QObject(parent),
	tcpServer(new QTcpServer(this)),
	tlsServer(this, allowedProtocols)
{
	if (allowedProtocols & Tls)
	{
		tcpServer = &tlsServer;
		QObject::connect(tcpServer, SIGNAL(newTlsConnection(QSslSocket*)), this, SLOT(newTlsConnection(QSslSocket*)));
	}
	else
	{
		QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()));
	}
}

QWsServer::~QWsServer()
{
	tcpServer->deleteLater();
}

bool QWsServer::listen(const QHostAddress & address, quint16 port)
{
	return tcpServer->listen(address, port);
}

void QWsServer::close()
{
	tcpServer->close();
}

Protocol QWsServer::allowedProtocols()
{
	return tlsServer.allowedProtocols();
}

QAbstractSocket::SocketError QWsServer::serverError()
{
	return tcpServer->serverError();
}

QString QWsServer::errorString()
{
	return tcpServer->errorString();
}

void QWsServer::newTcpConnection()
{
	QTcpSocket* tcpSocket = tcpServer->nextPendingConnection();
	if (tcpSocket == NULL)
	{
		return;
	}
	QObject::connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	QObject::connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()));
	handshakeBuffer.insert(tcpSocket, new QWsHandshake(WsClientMode));
}

void QWsServer::newTlsConnection(QSslSocket* serverSocket)
{
	if (serverSocket == NULL)
	{
		return;
	}
	QObject::connect(serverSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	QObject::connect(serverSocket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()));
	handshakeBuffer.insert(serverSocket, new QWsHandshake(WsClientMode));
}

void QWsServer::tcpSocketDisconnected()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == NULL)
	{
		return;
	}
	
	QWsHandshake* handshake = handshakeBuffer.take(tcpSocket);
	delete handshake;
	tcpSocket->deleteLater();
}

void QWsServer::closeTcpConnection()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == NULL)
	{
		return;
	}
	
	tcpSocket->close();
}

static void showErrorAndClose(QTcpSocket* tcpSocket)
{
	// Send bad request response
	QString response = QWsServer::composeBadRequestResponse(QList<EWebsocketVersion>() << WS_V6 << WS_V7 << WS_V8 << WS_V13);
	tcpSocket->write(response.toUtf8());
	tcpSocket->flush();
	tcpSocket->close();
}

void QWsServer::dataReceived()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == NULL)
	{
		return;
	}

	QWsHandshake& handshake = *(handshakeBuffer.value(tcpSocket));

	if (handshake.read(tcpSocket) == false)
	{
		showErrorAndClose(tcpSocket);
		return;
	}

	// handshake complete
	if (!handshake.readStarted || !handshake.complete)
	{
		if (handshake.readStarted && !handshake.httpRequestValid)
		{
			showErrorAndClose(tcpSocket);
		}
		return;
	}

	// If the mandatory fields are not specified, we abord the connection to the Websocket server
	// hansake valid
	if (!handshake.isValid())
	{
		showErrorAndClose(tcpSocket);
		return;
	}
	
	// Handshake fully parsed
	QObject::disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	QObject::disconnect(tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()));

	// Compose opening handshake response
	QByteArray handshakeResponse;

	if (handshake.version >= WS_V6)
	{
		QByteArray accept = QWsSocket::computeAcceptV4(handshake.key);
		handshakeResponse = QWsServer::composeOpeningHandshakeResponseV6(accept, handshake.protocol).toUtf8();
	}
	else if (handshake.version >= WS_V4)
	{
		QByteArray accept = QWsSocket::computeAcceptV4(handshake.key);
		QByteArray nonce = QWsSocket::generateNonce();
		handshakeResponse = QWsServer::composeOpeningHandshakeResponseV4(accept, nonce, handshake.protocol).toUtf8();
	}
	else // version WS_V0
	{
		QByteArray accept = QWsSocket::computeAcceptV0(handshake.key1, handshake.key2, handshake.key3);
		// safari 5.1.7 don't accept the utf8 charset here...
		handshakeResponse = QWsServer::composeOpeningHandshakeResponseV0(accept, handshake.origin, handshake.hostAddress, handshake.hostPort, handshake.resourceName , handshake.protocol).toLatin1();
	}
	
	// Send opening handshake response
	tcpSocket->write(handshakeResponse);
	tcpSocket->flush();

	QWsSocket* wsSocket = new QWsSocket(this, tcpSocket, handshake.version);
	wsSocket->setResourceName(handshake.resourceName);
	wsSocket->setHost(handshake.host);
	wsSocket->setHostAddress(handshake.hostAddress);
	wsSocket->setHostPort(handshake.hostPort.toUInt());
	wsSocket->setOrigin(handshake.origin);
	wsSocket->setProtocol(handshake.protocol);
	wsSocket->setExtensions(handshake.extensions);
	wsSocket->_wsMode = WsServerMode;
	
	QWsHandshake* hsTmp = handshakeBuffer.take(tcpSocket);
	delete hsTmp;

	// CAN'T DO THAT WITHOUT DISCONNECTING THE QTcpSocket
	//int socketDescriptor = tcpSocket->socketDescriptor();
	//incomingConnection(socketDescriptor);	
	// USE THIS INSTEAD
	addPendingConnection(wsSocket);
	emit newConnection();
}

void QWsServer::incomingConnection(int socketDescriptor)
{
	QTcpSocket* tcpSocket = new QTcpSocket(tcpServer);
	tcpSocket->setSocketDescriptor(socketDescriptor, QAbstractSocket::ConnectedState);
	QWsSocket* wsSocket = new QWsSocket(this, tcpSocket);

	addPendingConnection(wsSocket);
	emit newConnection();
}

void QWsServer::addPendingConnection(QWsSocket* socket)
{
	if (pendingConnections.size() < maxPendingConnections())
	{
		pendingConnections.enqueue(socket);
	}
}

QWsSocket* QWsServer::nextPendingConnection()
{
	return pendingConnections.dequeue();
}

bool QWsServer::hasPendingConnections()
{
	if (pendingConnections.size() > 0)
	{
		return true;
	}
	return false;
}

int QWsServer::maxPendingConnections()
{
	return tcpServer->maxPendingConnections();
}

bool QWsServer::isListening()
{
	return tcpServer->isListening();
}

QNetworkProxy QWsServer::proxy()
{
	return tcpServer->proxy();
}

QHostAddress QWsServer::serverAddress()
{
	return tcpServer->serverAddress();
}

quint16 QWsServer::serverPort()
{
	return tcpServer->serverPort();
}

void QWsServer::setMaxPendingConnections(int numConnections)
{
	tcpServer->setMaxPendingConnections(numConnections);
}

void QWsServer::setProxy(const QNetworkProxy & networkProxy)
{
	tcpServer->setProxy(networkProxy);
}

bool QWsServer::setSocketDescriptor(int socketDescriptor)
{
	return tcpServer->setSocketDescriptor(socketDescriptor);
}

int QWsServer::socketDescriptor()
{
	return tcpServer->socketDescriptor();
}

bool QWsServer::waitForNewConnection(int msec, bool* timedOut)
{
	return tcpServer->waitForNewConnection(msec, timedOut);
}

QString QWsServer::composeOpeningHandshakeResponseV0(QByteArray accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol)
{
	QString response;
	response += QLatin1String("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
	response += QLatin1String("Upgrade: Websocket\r\n");
	response += QLatin1String("Connection: Upgrade\r\n");
	response += QString("Sec-WebSocket-Origin: %1\r\n").arg(origin);
	if (!hostAddress.startsWith("ws://", Qt::CaseInsensitive))
	{
		hostAddress.prepend(QLatin1String("ws://"));
	}
	if (!hostPort.isEmpty())
	{
		hostPort.prepend(QLatin1Char(':'));
	}
	response += QString("Sec-WebSocket-Location: %1%2%3\r\n").arg(hostAddress).arg(hostPort).arg(resourceName);
	if (!protocol.isEmpty())
	{
		response += QString("Sec-WebSocket-Protocol: %1\r\n").arg(protocol);
	}
	response += QLatin1String("\r\n");
	response += QLatin1String(accept);

	return response;
}

QString QWsServer::composeOpeningHandshakeResponseV4(QByteArray accept, QByteArray nonce, QString protocol, QString extensions)
{
	QString response;
	response += QLatin1String("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
	response += QLatin1String("Upgrade: websocket\r\n");
	response += QLatin1String("Connection: Upgrade\r\n");
	response += QString("Sec-WebSocket-Accept: %1\r\n").arg(QLatin1String(accept));
	response += QString("Sec-WebSocket-Nonce: %1\r\n").arg(QLatin1String(nonce));
	if (!protocol.isEmpty())
	{
		response += QString("Sec-WebSocket-Protocol: %1\r\n").arg(protocol);
	}
	if (!extensions.isEmpty())
	{
		response += QString("Sec-WebSocket-Extensions: %1\r\n").arg(extensions);
	}
	response += QLatin1String("\r\n");
	return response;
}

QString QWsServer::composeOpeningHandshakeResponseV6(QByteArray accept, QString protocol, QString extensions)
{
	QString response;
	response += QLatin1String("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
	response += QLatin1String("Upgrade: websocket\r\n");
	response += QLatin1String("Connection: Upgrade\r\n");
	response += QString("Sec-WebSocket-Accept: %1\r\n").arg(QLatin1String(accept));
	if (!protocol.isEmpty())
	{
		response += QString("Sec-WebSocket-Protocol: %1\r\n").arg(protocol);
	}
	if (!extensions.isEmpty())
	{
		response += QString("Sec-WebSocket-Extensions: %1\r\n").arg(extensions);
	}
	response += QLatin1String("\r\n");
	return response;
}

QString QWsServer::composeBadRequestResponse(QList<EWebsocketVersion> versions)
{
	QString response;
	response += QLatin1String("HTTP/1.1 400 Bad Request\r\n");
	if (!versions.isEmpty())
	{
		QString versionsStr;
		int i = versions.size();
		while (i--)
		{
			versionsStr += QString::number((quint16)versions.at(i));
			if (i)
			{
				versionsStr += QLatin1String(", ");
			}
		}
		response += QString("Sec-WebSocket-Version: %1\r\n").arg(versionsStr);
	}
	return response;
}

} // namespace QtWebsocket
