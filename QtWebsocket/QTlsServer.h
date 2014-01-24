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
#include <QSslConfiguration>

namespace QtWebsocket
{

// This class manages basic and secured (TLS/SSL) TCP connections
class QTlsServer : public QTcpServer
{
	Q_OBJECT

	QSslConfiguration sslConfiguration;
	QList<QSslCertificate> caCertificates;
public:
	QTlsServer(const QSslConfiguration& sslConfiguration,
			   const QList<QSslCertificate>& caCertificates,
			   QObject* parent = 0);
	virtual ~QTlsServer();

private slots:
	void displayTlsErrors(const QList<QSslError>& errors);
	void tlsSocketEncrypted();

signals:
	void newTlsConnection(QSslSocket* serverSocket);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);
};

} // namespace QtWebsocket

#endif // QTLSSERVER_H
