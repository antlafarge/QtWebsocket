#include "QWsServer.h"

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

const QString QWsServer::regExpResourceNameStr( "GET\\s(.*)\\sHTTP/1.1\r\n" );
const QString QWsServer::regExpHostStr( "Host:\\s(.+:\\d+)\r\n" );
const QString QWsServer::regExpKeyStr( "Sec-WebSocket-Key:\\s(.{24})\r\n" );
const QString QWsServer::regExpVersionStr( "Sec-WebSocket-Version:\\s(\\d)\r\n" );
const QString QWsServer::regExpOriginStr( "Sec-WebSocket-Origin:\\s(.+)\r\n" );
const QString QWsServer::regExpProtocolStr( "Sec-WebSocket-Protocol:\\s(.+)\r\n" );
const QString QWsServer::regExpExtensionsStr( "Sec-WebSocket-Extensions:\\s(.+)\r\n" );

QWsServer::QWsServer(QObject * parent)
	: QObject(parent)
{
	tcpServer = new QTcpServer(this);

	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()));

	qsrand( QDateTime::currentMSecsSinceEpoch() );
}

QWsServer::~QWsServer()
{
	tcpServer->deleteLater();
}

bool QWsServer::listen(const QHostAddress & address, quint16 port)
{
	return tcpServer->listen(address, port);
}

void QWsServer::close()
{
	tcpServer->close();
}

QAbstractSocket::SocketError QWsServer::serverError()
{
	return tcpServer->serverError();
}

QString QWsServer::errorString()
{
	return tcpServer->errorString();
}

void QWsServer::newTcpConnection()
{
	QTcpSocket * clientSocket = tcpServer->nextPendingConnection();

	QObject * clientObject = qobject_cast<QObject*>(clientSocket);

	connect(clientObject, SIGNAL(readyRead()), this, SLOT(dataReceived()));
}

void QWsServer::dataReceived()
{
	QTcpSocket * clientSocket = qobject_cast<QTcpSocket*>(sender());
	if (clientSocket == 0)
		return;

	QString request( clientSocket->readAll() );

	QRegExp regExp;
	regExp.setMinimal( true );
	
	// Extract mandatory datas
	// Resource name
	regExp.setPattern( QWsServer::regExpResourceNameStr );
	regExp.indexIn(request);
	QString resourceName = regExp.cap(1);
	
	// Host (address & port)
	regExp.setPattern( QWsServer::regExpHostStr );
	regExp.indexIn(request);
	QStringList sl = regExp.cap(1).split(':');
	QString hostPort;
	if ( sl.size() > 1 )
		hostPort = sl[1];
	QString hostAddress = sl[0];
	
	// Key
	regExp.setPattern( QWsServer::regExpKeyStr );
	regExp.indexIn(request);
	QString key = regExp.cap(1);
	
	// Version
	regExp.setPattern( QWsServer::regExpVersionStr );
	regExp.indexIn(request);
	QString version = regExp.cap(1);
	
	// Extract optional datas
	// Origin
	regExp.setPattern( QWsServer::regExpOriginStr );
	regExp.indexIn(request);
	QString origin = regExp.cap(1);

	// Protocol
	regExp.setPattern( QWsServer::regExpProtocolStr );
	regExp.indexIn(request);
	QString protocol = regExp.cap(1);

	// Extensions
	regExp.setPattern( QWsServer::regExpExtensionsStr );
	regExp.indexIn(request);
	QString extensions = regExp.cap(1);

	// If the mandatory params are not setted, we abord the handshake
	if ( hostAddress.isEmpty()
		|| hostPort.isEmpty()
		|| resourceName.isEmpty()
		|| key.isEmpty()
		|| version != "8" )
		return;

	// Handshake OK, new connection
	disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	int socketDescriptor = clientSocket->socketDescriptor();
	incomingConnection( socketDescriptor );

	// Compose handshake answer
	QString accept = computeAcceptV8( key );
	
	QString answer("HTTP/1.1 101 Switching Protocols\r\n");
	answer.append("Upgrade: websocket\r\n");
	answer.append("Connection: Upgrade\r\n");
	answer.append("Sec-WebSocket-Accept: " + accept + "\r\n");
	answer.append("\r\n");

	// Send handshake answer
	clientSocket->write( answer.toUtf8() );
}

QString QWsServer::computeAcceptV8(QString key)
{
	key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	QByteArray hash = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1 );
	return hash.toBase64();
}

int QWsServer::maxPendingConnections()
{
	return tcpServer->maxPendingConnections();
}

void QWsServer::addPendingConnection( QWsSocket * socket )
{
	if ( pendingConnections.size() < maxPendingConnections() )
		pendingConnections.enqueue(socket);
}

void QWsServer::incomingConnection( int socketDescriptor )
{
	QTcpSocket * tcpSocket = new QTcpSocket;
	tcpSocket->setSocketDescriptor( socketDescriptor, QAbstractSocket::ConnectedState );
	QWsSocket * wsSocket = new QWsSocket( this, tcpSocket );

	addPendingConnection( wsSocket );

	emit newConnection();
}

bool QWsServer::hasPendingConnections()
{
	if ( pendingConnections.size() > 0 )
		return true;
	return false;
}

QWsSocket * QWsServer::nextPendingConnection()
{
	return pendingConnections.dequeue();
}

bool QWsServer::isListening()
{
	return tcpServer->isListening();
}

QNetworkProxy QWsServer::proxy()
{
	return tcpServer->proxy();
}

QHostAddress QWsServer::serverAddress()
{
	return tcpServer->serverAddress();
}

quint16 QWsServer::serverPort()
{
	return tcpServer->serverPort();
}

void QWsServer::setMaxPendingConnections( int numConnections )
{
	tcpServer->setMaxPendingConnections( numConnections );
}

void QWsServer::setProxy( const QNetworkProxy & networkProxy )
{
	tcpServer->setProxy( networkProxy );
}

bool QWsServer::setSocketDescriptor( int socketDescriptor )
{
	return tcpServer->setSocketDescriptor( socketDescriptor );
}

int QWsServer::socketDescriptor()
{
	return tcpServer->socketDescriptor();
}

bool QWsServer::waitForNewConnection( int msec, bool * timedOut )
{
	return tcpServer->waitForNewConnection( msec, timedOut );
}
