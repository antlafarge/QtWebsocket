#include "ServerThreaded.h"

#include "Log.h"

ServerThreaded::ServerThreaded()
{
	int port = 1337;
    server = new QWsServer( this );
	if ( ! server->listen( QHostAddress::Any, port ) )
	{
		Log::display( "Error: Can't launch server" );
		Log::display( "QWsServer error : " + server->errorString() );
	}
	else
	{
		Log::display( "Server is listening port " + QString::number(port) );
	}
	connect( server, SIGNAL(newConnection()), this, SLOT(processNewConnection()) );
}

ServerThreaded::~ServerThreaded()
{
}

void ServerThreaded::processNewConnection()
{
	Log::display("Client connected");

	// Get the connecting socket
    QWsSocket * socket = server->nextPendingWsConnection();

	// Create a new thread and giving to him the socket
    SocketThread *newClient = new SocketThread( socket );

    foreach (SocketThread *oldClients, socketThreads)
    {
        connect(oldClients, SIGNAL(messageReceived(QString)), newClient, SIGNAL(broadcastMessage(QString)));
        connect(newClient, SIGNAL(messageReceived(QString)), oldClients, SIGNAL(broadcastMessage(QString)));
    }

    connect(newClient, SIGNAL(messageReceived(QString)), this, SLOT(displayMessage(QString)));

    connect(newClient, SIGNAL(finished()), this, SLOT(deleteThread()));

    socketThreads << newClient;

    newClient->start();
}

void ServerThreaded::displayMessage(QString message)
{
    Log::display("New message: " + message);
}

void ServerThreaded::deleteThread()
{
    SocketThread *thread = qobject_cast<SocketThread*>(sender());
    if (thread == 0)
        return;
    socketThreads.removeOne(thread);
    thread->deleteLater();
    Log::display( "Client disconnected" );
}
