#include "ServerExample.h"

#include "Log.h"

ServerExample::ServerExample()
{
	int port = 1337;
    server = new QWsServer(this);
	if ( ! server->listen(QHostAddress::Any, port) )
	{
		Log::display( tr("Error: Can't launch server") );
		Log::display( tr("QWsServer error : %1").arg(server->errorString()) );
	}
	else
	{
		Log::display( tr("Server is listening on port %1").arg(port) );
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

ServerExample::~ServerExample()
{
}

void ServerExample::processNewConnection()
{
	QWsSocket * clientSocket = server->nextPendingConnection();

	connect( clientSocket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)) );
	connect( clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
	connect( clientSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)) );

	clients << clientSocket;

	Log::display(tr("Client connected"));
}

void ServerExample::processMessage( QString frame )
{
	QWsSocket * socket = qobject_cast<QWsSocket*>( sender() );
	if (socket == 0)
		return;

	Log::display( frame );
	
	QWsSocket * client;
	foreach ( client, clients )
	{
		client->write( frame );
	}
}

void ServerExample::processPong( quint64 elapsedTime )
{
	Log::display( tr("ping: %1 ms").arg(elapsedTime) );
}

void ServerExample::socketDisconnected()
{
	QWsSocket * socket = qobject_cast<QWsSocket*>( sender() );
	if (socket == 0)
		return;

	clients.removeOne(socket);

	socket->deleteLater();

	Log::display(tr("Client disconnected"));
}
