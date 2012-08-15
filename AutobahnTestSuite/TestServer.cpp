#include "QWsServer.h"
#include "QWsSocket.h"

#include "TestServer.h"

TestServer::TestServer() :
	server(new QWsServer(this))
{
	int port = 1337;
	if (!server->listen(QHostAddress::LocalHost, port))
	{
		qDebug("Error: Can't launch server");
		qDebug("QWsServer error: %s", server->errorString().toUtf8().constData());
	}
	else
	{
		qDebug("Server is listening port %d", port);
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(onClientConnection()));
}

TestServer::~TestServer()
{
}

void TestServer::onClientConnection()
{
	QWsSocket * clientSocket = server->nextPendingConnection();

	connect(clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(onDataReceived(QString)));
	connect(clientSocket, SIGNAL(frameReceived(QByteArray)), this, SLOT(onDataReceived(QByteArray)));
	connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onClientDisconnection()));
	connect(clientSocket, SIGNAL(pong(quint64)), this, SLOT(onPong(quint64)));

	clients << clientSocket;

	qDebug("Client connected");
}

void TestServer::onDataReceived(QString data)
{
	QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
	if (socket == 0)
		return;
	
	QWsSocket * client;
	foreach (client, clients)
	{
		client->write(data);
	}
}

void TestServer::onDataReceived(const QByteArray & data)
{
	QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
	if (socket == 0)
		return;

	QWsSocket * client;
	foreach (client, clients)
	{
		client->write(data);
	}
}

void TestServer::onPong(quint64 elapsedTime)
{
	qDebug("ping: %s ms", QString::number(elapsedTime).toAscii().constData());
}

void TestServer::onClientDisconnection()
{
	QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
	if (socket == 0)
		return;

	clients.removeOne(socket);

	socket->deleteLater();

	qDebug("Client disconnected");
}
