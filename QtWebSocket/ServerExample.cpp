#include "ServerExample.h"

#include "Log.h"

ServerExample::ServerExample()
{
    server = new QWsServer(this);
	if ( ! server->listen(QHostAddress::Any, 50885) )
		Log::display( "Error at server launch" );
	else
		Log::display( "server launched" );
	connect(server, SIGNAL(newConnection()), this, SLOT(onClientConnection()));
}

ServerExample::~ServerExample()
{

}

void ServerExample::onClientConnection()
{
    QWsSocket * clientSocket = server->nextPendingConnection();

	QObject * clientObject = qobject_cast<QObject*>(clientSocket);

    connect(clientObject, SIGNAL(frameReceived()), this, SLOT(onDataReceived()));
    connect(clientObject, SIGNAL(disconnected()), this, SLOT(onClientDisconnection()));
	
	clients << clientSocket;

	Log::display("Client connected");
}

void ServerExample::onDataReceived()
{
    QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
    if (socket == 0)
        return;

	Log::display("NbBytesReceived = " + QString::number(socket->bytesAvailable()) );

	QString msg = socket->readFrame();
	Log::display( "Message = " + msg );

	socket->write( msg.toUtf8() );

	if ( socket->bytesAvailable() )
		onDataReceived();
}

void ServerExample::onClientDisconnection()
{
    QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
    if (socket == 0)
        return;

	Log::display( "Error #" + QString::number( (int)socket->error() ) + ": " + socket->errorString() );

    clients.removeOne(socket);

    socket->deleteLater();

	Log::display("Client disconnected");
}
