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

#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include "QWsSocket.h"

namespace Ui {
class Client;
}

class Client : public QWidget
{
	Q_OBJECT

public:
    explicit Client(bool useTls, QWidget* parent = 0);
	~Client();

protected slots:
	void socketConnected();
	void socketEncrypted();
	void socketDisconnected();
	void sendMessage();
	void connectSocket();
	void disconnectSocket();
	void displayMessage(QString message);
	void socketStateChanged(QAbstractSocket::SocketState socketState);
	void displaySslErrors(const QList<QSslError>& errors);

protected:
	QtWebsocket::QWsSocket* wsSocket;
	QString defaultPseudo;

private:
	Ui::Client* ui;
};

#endif // CLIENT_H
