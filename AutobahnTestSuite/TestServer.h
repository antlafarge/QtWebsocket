#ifndef TESTSERVER_H
#define TESTSERVER_H

#include <QObject>
#include <QByteArray>
#include <QList>

#include "QWsServer.h"
#include "QWsSocket.h"

class TestServer : public QObject
{
	Q_OBJECT

public:
	TestServer(int port = 80);
	~TestServer();

public slots:
	void onClientConnection();
	void onDataReceived(QString data);
	void onDataReceived(const QByteArray & data);
	void onPong(quint64 elapsedTime);
	void onClientDisconnection();

private:
	QtWebsocket::QWsServer* server;
	QList<QtWebsocket::QWsSocket*> clients;
	int _port;
};

#endif
