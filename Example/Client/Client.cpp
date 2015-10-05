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

#include "Client.h"
#include "ui_Client.h"
#include <QInputDialog>
#include <QFile>
#include <QSslConfiguration>
#include <QSslKey>

Client::Client(bool useTls, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Client)
{
	ui->setupUi(this);

	defaultPseudo = QString("user%1").arg(qrand() % 9000 + 1000);
	ui->pseudoLineEdit->setPlaceholderText(defaultPseudo);

    if(useTls) // complex TLS configuration of websocket
    {
        QFile file("client-key.pem");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "cant load client key client-key.pem";
            throw -1;
        }
        QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray("qtwebsocket-client-key"));
        file.close();

        QFile file2("client-crt.pem");
        if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "cant load client certificate client-crt.pem";
            throw -2;
        }
        QSslCertificate localCert(&file2, QSsl::Pem);
        file2.close();

        QSslConfiguration sslConfiguration;
        sslConfiguration.setPrivateKey(key);
        sslConfiguration.setLocalCertificate(localCert);
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);

        QList<QSslCertificate> caCerts = QSslCertificate::fromPath("ca.pem");
        wsSocket = new QtWebsocket::QWsSocket(this, 0, QtWebsocket::WS_V13, &sslConfiguration, caCerts);
    }
    else // simple creation of QWsSocket without any hassle of TLS configuration.
        wsSocket = new QtWebsocket::QWsSocket(this);

	socketStateChanged(wsSocket->state());

	QObject::connect(ui->sendButton, SIGNAL(pressed()), this, SLOT(sendMessage()));
	QObject::connect(ui->textLineEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	QObject::connect(ui->connectButton, SIGNAL(pressed()), this, SLOT(connectSocket()));
	QObject::connect(ui->disconnectButton, SIGNAL(pressed()), this, SLOT(disconnectSocket()));
	QObject::connect(wsSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)));
	QObject::connect(wsSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
	QObject::connect(wsSocket, SIGNAL(encrypted()), this, SLOT(socketEncrypted()));
	QObject::connect(wsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(wsSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(displaySslErrors(const QList<QSslError>&)));
}

Client::~Client()
{
	delete ui;
}

void Client::sendMessage()
{
	QString pseudo = ui->pseudoLineEdit->text();
	pseudo = (pseudo.isEmpty() ? defaultPseudo : pseudo);

	QString message = ui->textLineEdit->text();
	message = (message.isEmpty() ? QLatin1String("echo") : message);

	ui->textLineEdit->clear();

	wsSocket->write(QString("%1: %2").arg(pseudo).arg(message));
}

void Client::displayMessage(QString message)
{
	ui->chatTextEdit->append(message);
}

void Client::displaySslErrors(const QList<QSslError>& errors)
{
	for (int i=0, sz=errors.size(); i<sz; i++)
	{
		QString errorString = errors.at(i).errorString();
		displayMessage(errorString);
	}
}

void Client::connectSocket()
{
	bool ok;
	QString ipAddress = QInputDialog::getText(this, tr("Client"), tr("Server IP:"), QLineEdit::Normal, "ws://localhost:80", &ok);
	ipAddress = ipAddress.trimmed();
	if (ok && !ipAddress.isEmpty())
	{
		QString ip;
		quint16 port;
		if (ipAddress.contains(QRegExp(QLatin1String(":([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{1,4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"))))
		{
			QStringList splitted = ipAddress.split(':');
			port = splitted.takeLast().toUInt();
			ip = splitted.join(':');
		}
		else
		{
			ip = ipAddress;
			port = 80;
		}
		wsSocket->connectToHost(ip.toUtf8(), port);
	}
}

void Client::disconnectSocket()
{
	wsSocket->disconnectFromHost();
}

void Client::socketConnected()
{	
	displayMessage(tr("CONNECTED"));
}

void Client::socketEncrypted()
{
	displayMessage(tr("ENCRYPTED"));
}

void Client::socketDisconnected()
{
	displayMessage(tr("DISCONNECTED"));
}

void Client::socketStateChanged(QAbstractSocket::SocketState socketState)
{
	switch (socketState)
	{
		case QAbstractSocket::UnconnectedState:
			ui->socketStateLabel->setText(tr("Unconnected"));
			break;
		case QAbstractSocket::HostLookupState:
			ui->socketStateLabel->setText(tr("HostLookup"));
			break;
		case QAbstractSocket::ConnectingState:
			ui->socketStateLabel->setText(tr("Connecting"));
			break;
		case QAbstractSocket::ConnectedState:
			ui->socketStateLabel->setText("Connected");
			break;
		case QAbstractSocket::BoundState:
			ui->socketStateLabel->setText(tr("Bound"));
			break;
		case QAbstractSocket::ClosingState:
			ui->socketStateLabel->setText(tr("Closing"));
			break;
		case QAbstractSocket::ListeningState:
			ui->socketStateLabel->setText(tr("Listening"));
			break;
		default:
			ui->socketStateLabel->setText(tr("Unknown"));
			break;
	}
}
