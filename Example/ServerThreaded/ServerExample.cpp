#include "ServerExample.h"

#include "Log.h"

ServerExample::ServerExample()
{
	int port = 1337;
    server = new QWsServer(this);
	if ( ! server->listen(QHostAddress::Any, port) )
	{
		Log::display( "Error: Can't launch server" );
		Log::display( "QWsServer error : " + server->errorString() );
	}
	else
	{
		Log::display( "Server is listening port " + QString::number(port) );
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

	SocketThread * st = new SocketThread();
	//Log::display( "wsSocket.thread before : " + QString::number((int)clientSocket->thread()) );
	//Log::display( "tcpSocket.thread before : " + QString::number((int)clientSocket->tcpSocket->thread()) );
	clientSocket->moveToThread( st );
	//Log::display( "wsSocket.thread after : " + QString::number((int)clientSocket->thread()) );
	//Log::display( "tcpSocket.thread after : " + QString::number((int)clientSocket->tcpSocket->thread()) );
	st->socket = clientSocket;
	st->start();

	Log::display("Client connected");
}
