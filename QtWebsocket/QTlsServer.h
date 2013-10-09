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

#ifndef QTLSSERVER_H
#define QTLSSERVER_H

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

namespace QtWebsocket
{

// This class manage basic and secured (TLS/SSL) TCP connections
class QTlsServer : public QTcpServer
{
	Q_OBJECT

public:
	QTlsServer(QObject* parent = NULL, Protocol allowedProtocols = Tcp);
	virtual ~QTlsServer();

	Protocol allowedProtocols();

public slots:
	void displayTlsErrors(const QList<QSslError>& errors);
	void tlsSocketEncrypted();
	void test();

signals:
	void newTlsConnection(QSslSocket* serverSocket);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);
	const Protocol _allowedProtocols;
};

} // namespace QtWebsocket

#endif // QTLSSERVER_H
