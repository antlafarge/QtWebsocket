#include "ServerExample.h"

#include "Log.h"

ServerExample::ServerExample()
{
    server = new QWsServer(this);
	if ( ! server->listen(QHostAddress::Any, 50885) )
	{
		Log::display( "Error at server launch" );
	}
	else
	{
		Log::display( "server launched" );
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(onClientConnection()));
}

ServerExample::~ServerExample()
{
}

void ServerExample::onClientConnection()
{
    QWsSocket * clientSocket = server->nextPendingConnection();

	QObject * clientObject = qobject_cast<QObject*>(clientSocket);

    connect(clientObject, SIGNAL(frameReceived(QString)), this, SLOT(onDataReceived(QString)));
    connect(clientObject, SIGNAL(disconnected()), this, SLOT(onClientDisconnection()));
	
	clients << clientSocket;

	Log::display("Client connected");
}

void ServerExample::onDataReceived(QString data)
{
    QWsSocket * socket = qobject_cast<QWsSocket*>( sender() );
    if (socket == 0)
        return;

	Log::display( "Message = " + data );

	socket->write( data.toUtf8() );
}

void ServerExample::onClientDisconnection()
{
    QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
    if (socket == 0)
        return;

    clients.removeOne(socket);

    socket->deleteLater();

	Log::display("Client disconnected");
}
