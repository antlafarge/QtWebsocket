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
	QWsSocket * socket = server->nextPendingConnection();

	// Create a new thread and giving to him the socket
	SocketThread * thread = new SocketThread( socket );
	
	// connect for message broadcast
	connect( socket, SIGNAL(frameReceived(QString)), this, SIGNAL(broadcastMessage(QString)) );
	connect( this, SIGNAL(broadcastMessage(QString)), thread, SLOT(sendMessage(QString)) );

	// connect for message display in log
	connect( socket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)) );

	// Starting the thread
	thread->start();
}

// Display the message received by a socket
void ServerThreaded::displayMessage( QString message )
{
	Log::display( message );
}	
