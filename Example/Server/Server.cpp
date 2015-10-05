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

#include "Server.h"
#include <qDebug>

Server::Server(int port, QtWebsocket::Protocol protocol)
{
	if(protocol == QtWebsocket::Tcp)
		server = new QtWebsocket::QWsServer(this, protocol);
	else
	{
		QFile file("server-key.pem");
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "can't open key server-key.pem";
			throw -1;
		}
		QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray("qtwebsocket-server-key"));
		file.close();

		QFile file2("server-crt.pem");
		if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "cant load server certificate server-crt.pem";
			throw -2;
		}
		QSslCertificate localCert(&file2, QSsl::Pem);
		file2.close();

		QSslConfiguration sslConfiguration;
		sslConfiguration.setPrivateKey(key);
		sslConfiguration.setLocalCertificate(localCert);
		sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);

		QList<QSslCertificate> caCerts = QSslCertificate::fromPath("ca.pem");
		server = new QtWebsocket::QWsServer(this, protocol, sslConfiguration, caCerts);
	}
	if (! server->listen(QHostAddress::Any, port))
	{
		qDebug() << tr("Error: Can't launch server");
		qDebug() << tr("QWsServer error : %1").arg(server->errorString());
	}
	else
	{
		qDebug() << tr("Server is listening on port %1").arg(port);
	}
	QObject::connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

Server::~Server()
{
}

void Server::processNewConnection()
{
	QtWebsocket::QWsSocket* clientSocket = server->nextPendingConnection();

	QObject::connect(clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)));
	QObject::connect(clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(clientSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	clients << clientSocket;

	qDebug() << tr("Client connected (%1)").arg(clientSocket->isEncrypted() ? "encrypted" : "not encrypted");
}

QString toReadableAscii(QString string)
{
    // We must not use QLatin1String
    // It cannot decode multibyte Latin-1 characters when source file encoding is UTF-8.
    string.replace(QRegExp(QString("[ÀÁÂÃÄÅ]")), "ÀÁÂÃÄÅ");
    string.replace(QRegExp(QString("[àáâãäåª]")), "a");
    string.replace(QRegExp(QString("[ÈÉÊË£]")), "E");
    string.replace(QRegExp(QString("[èéêë]")), "e");
    string.replace(QRegExp(QString("[ÌÍÎÏ]")), "I");
    string.replace(QRegExp(QString("[ìíîï¡]")), "i");
    string.replace(QRegExp(QString("[ÒÓÔÕÖØ]")), "O");
    string.replace(QRegExp(QString("[òóôõöðø¤°º]")), "o");
    string.replace(QRegExp(QString("[ÙÚÛÜ]")), "U");
    string.replace(QRegExp(QString("[ùúûüµ]")), "u");
    string.replace(QRegExp(QString("[¥Ý]")), "Y");
    string.replace(QRegExp(QString("[ýÿ]")), "y");
    string.replace(QRegExp(QString("[Ç¢]")), "C");
    // We must use UTF-16 literals
    // because Latin-1 characters are multibyte when source file encoding is UTF-8.
    string.replace(QChar(L'ç'), "c");
    string.replace(QChar(L'©'), "(C)");
    string.replace(QChar(L'®'), "(R)");
    string.replace(QChar(L'«'), "<<");
    string.replace(QChar(L'»'), ">>");
    string.replace(QChar(L'¦'), "|");
    string.replace(QChar(L'§'), "S");
    string.replace(QChar(L'¨'), "\"");
    string.replace(QChar(L'¬'), "-");
    string.replace(QChar(L'-'), "-");
    string.replace(QChar(L'¯'), "-");
    string.replace(QChar(L'¹'), "^1");
    string.replace(QChar(L'²'), "^2");
    string.replace(QChar(L'³'), "^3");
    string.replace(QChar(L'±'), "+-");
    string.replace(QChar(L'¼'), "1/4");
    string.replace(QChar(L'½'), "1/2");
    string.replace(QChar(L'¾'), "3/4");
    string.replace(QChar(L'×'), "x");
    string.replace(QChar(L'÷'), "/");
    string.replace(QChar(L'´'), "`");
    string.replace(QChar(L'·'), ".");
    string.replace(QChar(L'¸'), ",");
    string.replace(QChar(L'¿'), "?");
    string.replace(QChar(L'¶'), "g");
    string.replace(QChar(L'Æ'), "AE");
    string.replace(QChar(L'æ'), "ae");
    string.replace(QChar(L'Ð'), "D");
    string.replace(QChar(L'Ñ'), "N");
    string.replace(QChar(L'ñ'), "n");
    string.replace(QChar(L'Þ'), "D");
    string.replace(QChar(L'þ'), "d");
    string.replace(QChar(L'ß'), "B");
    string.replace(QChar(0x20AC), "E"); // euro
	int i = string.size();
	while (i--)
	{
		QChar c = string.at(i);
		if (c < 32 || c > 126)
		{
			string[i] = ' ';
		}
	}
	return string;
}

void Server::processMessage(QString frame)
{
	QtWebsocket::QWsSocket* socket = qobject_cast<QtWebsocket::QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}
	qDebug() << toReadableAscii(frame);
	
	QtWebsocket::QWsSocket* client;
	foreach (client, clients)
	{
		client->write(frame);
	}
}

void Server::processPong(quint64 elapsedTime)
{
	qDebug() << tr("ping: %1 ms").arg(elapsedTime);
}

void Server::socketDisconnected()
{
	QtWebsocket::QWsSocket* socket = qobject_cast<QtWebsocket::QWsSocket*>(sender());
	if (socket == 0)
	{
		return;
	}

	clients.removeOne(socket);

	socket->deleteLater();

	qDebug() << tr("Client disconnected");
}
