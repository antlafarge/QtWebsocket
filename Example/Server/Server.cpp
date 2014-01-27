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
	if (! server->listen(QHostAddress::Any, port, protocol))
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
	string.replace(QRegExp(QLatin1String("[ÀÁÂÃÄÅ]")), "ÀÁÂÃÄÅ");
	string.replace(QRegExp(QLatin1String("[àáâãäåª]")), "a");
	string.replace(QRegExp(QLatin1String("[ÈÉÊË£]")), "E");
	string.replace(QRegExp(QLatin1String("[èéêë]")), "e");
	string.replace(QRegExp(QLatin1String("[ÌÍÎÏ]")), "I");
	string.replace(QRegExp(QLatin1String("[ìíîï¡]")), "i");
	string.replace(QRegExp(QLatin1String("[ÒÓÔÕÖØ]")), "O");
	string.replace(QRegExp(QLatin1String("[òóôõöðø¤°º]")), "o");
	string.replace(QRegExp(QLatin1String("[ÙÚÛÜ]")), "U");
	string.replace(QRegExp(QLatin1String("[ùúûüµ]")), "u");
	string.replace(QRegExp(QLatin1String("[¥Ý]")), "Y");
	string.replace(QRegExp(QLatin1String("[ýÿ]")), "y");
	string.replace(QRegExp(QLatin1String("[Ç¢]")), "C");
	string.replace(QLatin1Char('ç'), "c");
	string.replace(QLatin1Char('©'), "(C)");
	string.replace(QLatin1Char('®'), "(R)");
	string.replace(QLatin1Char('«'), "<<");
	string.replace(QLatin1Char('»'), ">>");
	string.replace(QLatin1Char('¦'), "|");
	string.replace(QLatin1Char('§'), "S");
	string.replace(QLatin1Char('¨'), "\"");
	string.replace(QLatin1Char('¬'), "-");
	string.replace(QLatin1Char('-'), "-");
	string.replace(QLatin1Char('¯'), "-");
	string.replace(QLatin1Char('¹'), "^1");
	string.replace(QLatin1Char('²'), "^2");
	string.replace(QLatin1Char('³'), "^3");
	string.replace(QLatin1Char('±'), "+-");
	string.replace(QLatin1Char('¼'), "1/4");
	string.replace(QLatin1Char('½'), "1/2");
	string.replace(QLatin1Char('¾'), "3/4");
	string.replace(QLatin1Char('×'), "x");
	string.replace(QLatin1Char('÷'), "/");
	string.replace(QLatin1Char('´'), "`");
	string.replace(QLatin1Char('·'), ".");
	string.replace(QLatin1Char('¸'), ",");
	string.replace(QLatin1Char('¿'), "?");
	string.replace(QLatin1Char('¶'), "g");
	string.replace(QLatin1Char('Æ'), "AE");
	string.replace(QLatin1Char('æ'), "ae");
	string.replace(QLatin1Char('Ð'), "D");
	string.replace(QLatin1Char('Ñ'), "N");
	string.replace(QLatin1Char('ñ'), "n");
	string.replace(QLatin1Char('Þ'), "D");
	string.replace(QLatin1Char('þ'), "d");
	string.replace(QLatin1Char('ß'), "B");
	string.replace(QChar(0x20AC), "E");
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
