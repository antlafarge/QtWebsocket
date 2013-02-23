#include "SocketThread.h"

#include "Log.h"

SocketThread::SocketThread( QWsSocket * wsSocket ) :
	socket( wsSocket )
{
	// Set this thread as parent of the socket
	// This will push the socket in the good thread when using moveToThread on the parent
	if ( socket )
    {
		socket->setParent( this );
    }

	// Move this thread object in the thread himsleft
	// Thats necessary to exec the event loop in this thread
	moveToThread( this );
}

SocketThread::~SocketThread()
{

}

void SocketThread::run()
{
    Log::display( "connect done in thread : 0x" + QString::number((unsigned int)QThread::currentThreadId(), 16) );

	// Connecting the socket signals here to exec the slots in the new thread
	connect( socket, SIGNAL(frameReceived(QString)), this, SLOT(processMessage(QString)) );
	connect( socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
	connect( socket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)) );

	// Launch the event loop to exec the slots
	exec();
}

void SocketThread::processMessage( QString message )
{
    // ANY PROCESS HERE IS DONE IN THE SOCKET THREAD !

    Log::display( "thread 0x" + QString::number((unsigned int)QThread::currentThreadId(), 16) + " | " + message );
}

void SocketThread::sendMessage( QString message )
{
	socket->write( message );
}

void SocketThread::processPong( quint64 elapsedTime )
{
	Log::display( "ping: " + QString::number(elapsedTime) + " ms" );
}

void SocketThread::socketDisconnected()
{
	Log::display("Client disconnected, thread finished");

	// Prepare the socket to be deleted after last events processed
	socket->deleteLater();

	// finish the thread execution (that quit the event loop launched by exec)
	quit();
}
