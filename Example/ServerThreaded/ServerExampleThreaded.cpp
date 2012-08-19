#include "ServerExampleThreaded.h"

#include "Log.h"

ServerExampleThreaded::ServerExampleThreaded()
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

ServerExampleThreaded::~ServerExampleThreaded()
{
}

void ServerExampleThreaded::processNewConnection()
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

void ServerExampleThreaded::displayMessage( QString message )
{
	// Just display in log the message received by a socket
	Log::display( QString::fromUtf8( message.toStdString().c_str() ) );
}	
