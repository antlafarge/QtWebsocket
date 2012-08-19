#include "SocketThread.h"

#include "Log.h"

SocketThread::SocketThread()
{

}

SocketThread::~SocketThread()
{

}

void SocketThread::run()
{
	Log::display( "connect in thread : " + QString::number((int)QThread::currentThreadId()) );

	connect( socket, SIGNAL(frameReceived(QString)), this, SLOT(onDataReceived(QString)), Qt::DirectConnection );
	connect( socket, SIGNAL(disconnected()), this, SLOT(onClientDisconnection()), Qt::DirectConnection );
	connect( socket, SIGNAL(pong(quint64)), this, SLOT(onPong(quint64)), Qt::DirectConnection );

	exec();
}

void SocketThread::onDataReceived(QString data)
{
	Log::display( "dataReceived 2 in thread : " + QString::number((int)QThread::currentThreadId()) );

	Log::display( QString(data.toLatin1()) );
	
	socket->write( "your thread is " + QString::number( (int)QThread::currentThreadId() ) );
}

void SocketThread::onPong(quint64 elapsedTime)
{
	Log::display( "ping: " + QString::number(elapsedTime) + " ms" );
}

void SocketThread::onClientDisconnection()
{
	socket->deleteLater();

	Log::display("Client disconnected");

	quit();
}
