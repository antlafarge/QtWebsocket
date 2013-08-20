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

#include "QWsServer.h"

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

QRegExp QWsServer::regExpHttpRequest(QLatin1String("^GET\\s(.*)\\sHTTP/1.1\\r\\n"));
QRegExp QWsServer::regExpHost(QLatin1String("\\r\\nHost:\\s(([^:]+)(:([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{1,4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?)\\r\\n"));
QRegExp QWsServer::regExpKey(QLatin1String("\\r\\nSec-WebSocket-Key:\\s(.{24})\\r\\n"));
QRegExp QWsServer::regExpKey1(QLatin1String("\\r\\nSec-WebSocket-Key1:\\s(.+)\\r\\n"));
QRegExp QWsServer::regExpKey2(QLatin1String("\\r\\nSec-WebSocket-Key2:\\s(.+)\\r\\n"));
QRegExp QWsServer::regExpVersion(QLatin1String("\\r\\nSec-WebSocket-Version:\\s(\\d+)\\r\\n"));
QRegExp QWsServer::regExpOrigin(QLatin1String("\\r\\n(Origin|Sec-WebSocket-Origin):\\s(.+)\\r\\n"));
QRegExp QWsServer::regExpProtocol(QLatin1String("\\r\\nSec-WebSocket-Protocol:\\s(.+)\\r\\n"));
QRegExp QWsServer::regExpExtensions(QLatin1String("\\r\\nSec-WebSocket-Extensions:\\s(.+)\\r\\n"));

QWsServer::QWsServer(QObject* parent)
	: QObject(parent)
{
	tcpServer = new QTcpServer(this);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()));
	qsrand(QDateTime::currentMSecsSinceEpoch());
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
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	headerBuffer.insert(tcpSocket, new QString());
}

void QWsServer::disconnected()
{
	QTcpSocket* tcpSocket = (QTcpSocket *)sender();
	headerBuffer.remove(tcpSocket);
	tcpSocket->deleteLater();
}

void QWsServer::closeTcpConnection()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == 0)
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

static const QLatin1String emptyLine("\r\n");

void QWsServer::dataReceived()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == 0)
	{
		return;
	}

	bool allHeadersFetched = false;

	QString& request = *(headerBuffer[tcpSocket]);
	while (tcpSocket->canReadLine())
	{
		QString line = tcpSocket->readLine();
		request.append(line);
		if (line == emptyLine)
		{
			allHeadersFetched = true;
			break;
		}
		if (line.size()>1000) // incase of garbage input
		{
			showErrorAndClose(tcpSocket);
			return;
		}
	}
	if (!allHeadersFetched)
	{
		if (request.size()>10000) // incase of garbage input
		{
			showErrorAndClose(tcpSocket);
		}
		return;
	}

	// Extract mandatory fields
	
	bool missingField = false;
	
	// HTTP REQUEST and Resource name
	regExpHttpRequest.setMinimal(true);
	if (regExpHttpRequest.indexIn(request) == -1)
	{
		missingField = true;
	}
	QString resourceName = regExpHttpRequest.cap(1);
	
	// Host (address & port)
	regExpHost.setMinimal(true);
	if (regExpHost.indexIn(request) == -1)
	{
		missingField = true;
	}
	QString host = regExpHost.cap(1);
	QString hostAddress = regExpHost.cap(2);
	QString hostPort = regExpHost.cap(4);
	
	// Version
	QByteArray key3;
	EWebsocketVersion version;
	regExpVersion.setMinimal(true);
	if (regExpVersion.indexIn(request) == -1)
	{
		if (tcpSocket->bytesAvailable() == 8)
		{
			version = WS_V0;
			key3 = tcpSocket->read(8);
			request += key3;
		}
		else
		{
			version = WS_VUnknow;
		}
	}
	else
	{
		version = (EWebsocketVersion)regExpVersion.cap(1).toUInt();
	}

	// Key
	QByteArray key, key1, key2;
	if (version >= WS_V4)
	{
		regExpKey.setMinimal(true);
		regExpKey.indexIn(request);
		key = regExpKey.cap(1).toUtf8();
	}
	else
	{
		regExpKey1.setMinimal(true);
		regExpKey1.indexIn(request);
		key1 = regExpKey1.cap(1).toLatin1();

		regExpKey2.setMinimal(true);
		regExpKey2.indexIn(request);
		key2 = regExpKey2.cap(1).toLatin1();
	}
	
	////////////////////////////////////////////////////////////////

	// If the mandatory fields are not specified, we abord the connection to the Websocket server
	if ( missingField
		|| version == WS_VUnknow
		|| resourceName.isEmpty()
		|| hostAddress.isEmpty()
		|| ((key.isEmpty() && (key1.isEmpty() || key2.isEmpty() || key3.isEmpty()))) )
	{
		showErrorAndClose(tcpSocket);
		return;
	}
	
	////////////////////////////////////////////////////////////////
	
	// Extract optional fields

	// Origin
	regExpOrigin.setMinimal(true);
	regExpOrigin.indexIn(request);
	QString origin = regExpOrigin.cap(2);

	// Protocol
	regExpProtocol.setMinimal(true);
	regExpProtocol.indexIn(request);
	QString protocol = regExpProtocol.cap(1);

	// Extensions
	regExpExtensions.setMinimal(true);
	regExpExtensions.indexIn(request);
	QString extensions = regExpExtensions.cap(1);
	
	////////////////////////////////////////////////////////////////
	
	// Handshake fully parsed
	QObject::disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	QObject::disconnect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	headerBuffer.remove(tcpSocket);

	// Compose opening handshake response
	QString response;

	if (version >= WS_V6)
	{
		QByteArray accept = computeAcceptV4(key);
		response = QWsServer::composeOpeningHandshakeResponseV6(accept, protocol);
	}
	else if (version >= WS_V4)
	{
		QByteArray accept = computeAcceptV4(key);
		QByteArray nonce = generateNonce();
		response = QWsServer::composeOpeningHandshakeResponseV4(accept, nonce, protocol);
	}
	else
	{
		QByteArray accept = computeAcceptV0(key1, key2, key3);
		response = QWsServer::composeOpeningHandshakeResponseV0(accept, origin, hostAddress, hostPort, resourceName , protocol);
	}

	// Send opening handshake response
	if (version == WS_V0)
	{
		tcpSocket->write(response.toLatin1());
	}
	else
	{
		tcpSocket->write(response.toUtf8());
	}
	tcpSocket->flush();

	QWsSocket* wsSocket = new QWsSocket(this, tcpSocket, version);
	wsSocket->setResourceName(resourceName);
	wsSocket->setHost(host);
	wsSocket->setHostAddress(hostAddress);
	wsSocket->setHostPort(hostPort.toInt());
	wsSocket->setOrigin(origin);
	wsSocket->setProtocol(protocol);
	wsSocket->setExtensions(extensions);
	wsSocket->serverSideSocket = true;
	
	// ORIGINAL CODE
	//int socketDescriptor = tcpSocket->socketDescriptor();
	//incomingConnection(socketDescriptor);
	
	// CHANGED CODE FOR LINUX COMPATIBILITY
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

QByteArray QWsServer::computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray key3)
{
	quint32 key_number_1 = QString::fromLatin1(key1).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();
	quint32 key_number_2 = QString::fromLatin1(key2).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();

	int spaces_1 = key1.count(' ');
	int spaces_2 = key2.count(' ');

	quint32 part_1 = key_number_1 / spaces_1;
	quint32 part_2 = key_number_2 / spaces_2;

	QByteArray challenge;
	QDataStream ds(&challenge, QIODevice::WriteOnly);
	ds << part_1 << part_2;
	challenge.append(key3);

	QByteArray md5 = QCryptographicHash::hash(challenge, QCryptographicHash::Md5);

	return md5;
}

QByteArray QWsServer::computeAcceptV4(QByteArray key)
{
	key += QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	QByteArray hash = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
	return hash.toBase64();
}

QByteArray QWsServer::generateNonce()
{
	qsrand(QDateTime::currentDateTime().toTime_t());

	QByteArray nonce;

	int i = 16;
	while(i--)
	{
		nonce.append(qrand() % 0x100);
	}

	return nonce.toBase64();
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
