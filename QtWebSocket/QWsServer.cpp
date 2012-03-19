#include "QWsServer.h"

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

const QString QWsServer::regExpResourceNameStr( "GET\\s(.*)\\sHTTP/1.1\r\n" );
const QString QWsServer::regExpHostStr( "Host:\\s(.+(:\\d+)?)\r\n" );
const QString QWsServer::regExpKeyStr( "Sec-WebSocket-Key:\\s(.{24})\r\n" );
const QString QWsServer::regExpKey1Str( "Sec-WebSocket-Key1:\\s(.+)\r\n" );
const QString QWsServer::regExpKey2Str( "Sec-WebSocket-Key2:\\s(.+)\r\n" );
const QString QWsServer::regExpKey3Str( "(.{8})(\r\n)*$" );
const QString QWsServer::regExpVersionStr( "Sec-WebSocket-Version:\\s(\\d+)\r\n" );
const QString QWsServer::regExpOriginStr( "Origin:\\s(.+)\r\n" );
const QString QWsServer::regExpOriginV6Str( "Sec-WebSocket-Origin:\\s(.+)\r\n" );
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
	
	// Version
	regExp.setPattern( QWsServer::regExpVersionStr );
	regExp.indexIn(request);
	QString versionStr = regExp.cap(1);
	int version = 0;
	if ( ! versionStr.isEmpty() )
		version = versionStr.toInt();
	
	// Resource name
	regExp.setPattern( QWsServer::regExpResourceNameStr );
	regExp.indexIn(request);
	QString resourceName = regExp.cap(1);
	
	// Host (address & port)
	regExp.setPattern( QWsServer::regExpHostStr );
	regExp.indexIn(request);
	QStringList sl = regExp.cap(1).split(':');
	QString hostAddress = sl[0];
	QString hostPort;
	if ( sl.size() > 1 )
		hostPort = sl[1];
	
	// Key
	QString key, key1, key2, key3;
	if ( version >= 6 )
	{
		regExp.setPattern( QWsServer::regExpKeyStr );
		regExp.indexIn(request);
		key = regExp.cap(1);
	}
	else
	{
		regExp.setPattern( QWsServer::regExpKey1Str );
		regExp.indexIn(request);
		key1 = regExp.cap(1);
		regExp.setPattern( QWsServer::regExpKey2Str );
		regExp.indexIn(request);
		key2 = regExp.cap(1);
		regExp.setPattern( QWsServer::regExpKey3Str );
		regExp.indexIn(request);
		key3 = regExp.cap(1);
	}
	
	// Extract optional datas
	// Origin
	QString origin;
	if ( version < 6 || version > 8 )
	{
		regExp.setPattern( QWsServer::regExpOriginStr );
		regExp.indexIn(request);
		origin = regExp.cap(1);
	}
	else
	{
		regExp.setPattern( QWsServer::regExpOriginV6Str );
		regExp.indexIn(request);
		origin = regExp.cap(1);
	}

	// Protocol
	regExp.setPattern( QWsServer::regExpProtocolStr );
	regExp.indexIn(request);
	QString protocol = regExp.cap(1);

	// Extensions
	regExp.setPattern( QWsServer::regExpExtensionsStr );
	regExp.indexIn(request);
	QString extensions = regExp.cap(1);
	
	////////////////////////////////////////////////////////////////////

	if ( version < 6 )
	{
		qDebug() << "======== Handshake Received \n"
				 << request
				 << "======== \n";
	}

	// If the mandatory params are not setted, we abord the connection to the Websocket server
	if ( hostAddress.isEmpty()
		|| resourceName.isEmpty()
		|| ( key.isEmpty() && ( key1.isEmpty() || key2.isEmpty() || key3.isEmpty() ) )
	   )
		return;
	
	////////////////////////////////////////////////////////////////////
	
	// Compose handshake answer
	
	QString answer;
	
	QString accept;
	if ( version >= 6 )
	{
		accept = computeAcceptV2( key );
		answer.append("HTTP/1.1 101 Switching Protocols\r\n");
		answer.append("Upgrade: websocket\r\n");
		answer.append("Connection: Upgrade\r\n");
		answer.append("Sec-WebSocket-Accept: " + accept + "\r\n" + "\r\n");
	}
	else if ( version < 6 )
	{
		accept = computeAcceptV1( key1, key2, key3 );
		answer.append("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
		answer.append("Upgrade: Websocket\r\n");
		answer.append("Connection: Upgrade\r\n");
		answer.append("Sec-WebSocket-Origin: " + origin + "\r\n");
		answer.append("Sec-WebSocket-Location: ws://" + hostAddress + ( hostPort.isEmpty() ? "" : (":"+hostPort) ) + resourceName + "\r\n");
		if ( !protocol.isEmpty() )
			answer.append("Sec-WebSocket-Protocol: " + protocol + "\r\n");
		answer.append( accept );
	}
	
	if ( version < 6 )
	{
		qDebug() << "======== Handshake sent \n"
				 << answer
				 << "======== \n";
	}

	// Handshake OK, new connection
	disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));

	// Send handshake answer
	clientSocket->write( answer.toUtf8() );
	clientSocket->flush();

	// TEMPORARY CODE FOR LINUX COMPATIBILITY
	QWsSocket * wsSocket = new QWsSocket( clientSocket, this );
	addPendingConnection( wsSocket );
	emit newConnection();

	/*
	// ORIGINAL CODE
	int socketDescriptor = clientSocket->socketDescriptor();
	incomingConnection( socketDescriptor );
	*/
}

void QWsServer::incomingConnection( int socketDescriptor )
{
	QTcpSocket * tcpSocket = new QTcpSocket(tcpServer);
	tcpSocket->setSocketDescriptor( socketDescriptor, QAbstractSocket::ConnectedState );
	QWsSocket * wsSocket = new QWsSocket( tcpSocket, this );

	addPendingConnection( wsSocket );
	emit newConnection();
}

void QWsServer::addPendingConnection( QWsSocket * socket )
{
	if ( pendingConnections.size() < maxPendingConnections() )
		pendingConnections.enqueue(socket);
}

QWsSocket * QWsServer::nextPendingConnection()
{
	return pendingConnections.dequeue();
}

bool QWsServer::hasPendingConnections()
{
	if ( pendingConnections.size() > 0 )
		return true;
	return false;
}

int QWsServer::maxPendingConnections()
{
	return tcpServer->maxPendingConnections();
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

QString QWsServer::computeAcceptV2(QString key)
{
	key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	QByteArray hash = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1 );
	return hash.toBase64();
}

QString QWsServer::computeAcceptV1( QString key1, QString key2, QString key3 )
{
	QString numStr1;
	QString numStr2;

	QChar carac;
	for ( int i=0 ; i<key1.size() ; i++ )
	{
		carac = key1[ i ];
		if ( carac.isDigit() )
			numStr1.append( carac );
	}
	for ( int i=0 ; i<key2.size() ; i++ )
	{
	    carac = key2[ i ];
		if ( carac.isDigit() )
			numStr2.append( carac );
	}

	quint32 num1 = numStr1.toUInt();
	quint32 num2 = numStr2.toUInt();

	qDebug() << QString::number(num1);
	qDebug() << QString::number(num2);

	int numSpaces1 = key1.count( ' ' );
	int numSpaces2 = key2.count( ' ' );

	qDebug() << QString::number(numSpaces1);
	qDebug() << QString::number(numSpaces2);

	num1 /= numSpaces1;
	num2 /= numSpaces2;

	QString concat = serializeInt( num1 ) + serializeInt( num2 ) + key3;

	QByteArray md5 = QCryptographicHash::hash( concat.toAscii(), QCryptographicHash::Md5 );
  
	return QString( md5 );
}

QString QWsServer::serializeInt( quint32 number, quint8 nbBytes )
{
	QString bin;
	quint8 currentNbBytes = 0;
	while (number > 0 && currentNbBytes < nbBytes)
	{  
		bin.prepend( QChar::fromAscii(number) );
		number = number >> 8;
		currentNbBytes++;
	}
	while (currentNbBytes < nbBytes)
	{
		bin.prepend( QChar::fromAscii(0) );
		currentNbBytes++;
    }
	return bin;
}
