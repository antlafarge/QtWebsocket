#include "QWsServer.h"

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

const QString QWsServer::regExpResourceNameStr( "^GET\\s(.*)\\sHTTP/1.1\r\n" );
const QString QWsServer::regExpHostStr( "\r\nHost:\\s(.+(:\\d+)?)\r\n" );
const QString QWsServer::regExpKeyStr( "\r\nSec-WebSocket-Key:\\s(.{24})\r\n" );
const QString QWsServer::regExpKey1Str( "\r\nSec-WebSocket-Key1:\\s(.+)\r\n" );
const QString QWsServer::regExpKey2Str( "\r\nSec-WebSocket-Key2:\\s(.+)\r\n" );
const QString QWsServer::regExpKey3Str( "\r\n(.{8})$" );
const QString QWsServer::regExpVersionStr( "\r\nSec-WebSocket-Version:\\s(\\d+)\r\n" );
const QString QWsServer::regExpOriginStr( "\r\nSec-WebSocket-Origin:\\s(.+)\r\n" );
const QString QWsServer::regExpOrigin2Str( "\r\nOrigin:\\s(.+)\r\n" );
const QString QWsServer::regExpProtocolStr( "\r\nSec-WebSocket-Protocol:\\s(.+)\r\n" );
const QString QWsServer::regExpExtensionsStr( "\r\nSec-WebSocket-Extensions:\\s(.+)\r\n" );

QWsServer::QWsServer(QObject * parent)
	: QObject(parent)
{
	tcpServer = new QTcpServer(this);
	connect( tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()) );
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
	connect( clientObject, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
	headerBuffer.insert(clientSocket, QStringList());
}

void QWsServer::closeTcpConnection()
{
	QTcpSocket * clientSocket = qobject_cast<QTcpSocket*>( sender() );
	if (clientSocket == 0)
		return;

	clientSocket->close();
}

void QWsServer::dataReceived()
{
	QTcpSocket * clientSocket = qobject_cast<QTcpSocket*>( sender() );
	if (clientSocket == 0)
		return;

	bool allHeadersFetched = false;

	const QLatin1String emptyLine("\r\n");

	while (clientSocket->canReadLine()) {
		QString line = clientSocket->readLine();

		if (line == emptyLine) {
			allHeadersFetched = true;
			break;
		}

		headerBuffer[clientSocket].append(line);
	}

	if (!allHeadersFetched)
	    return;

	QString request( headerBuffer[clientSocket].join("") );

	QRegExp regExp;
	regExp.setMinimal( true );
	
	// Extract mandatory datas
	EWebsocketVersion version2 = (EWebsocketVersion)14;
	
	// Version
	regExp.setPattern( QWsServer::regExpVersionStr );
	regExp.indexIn(request);
	QString versionStr = regExp.cap(1);
	EWebsocketVersion version;
	if ( ! versionStr.isEmpty() )
	{
		version = (EWebsocketVersion)versionStr.toInt();
	}
	else if ( clientSocket->bytesAvailable() >= 8 )
	{
		version = WS_V0;
		request.append( clientSocket->read(8) );
	}
	else
	{
		version = WS_VUnknow;
	}

	// Resource name
	regExp.setPattern( QWsServer::regExpResourceNameStr );
	regExp.indexIn(request);
	QString resourceName = regExp.cap(1);
	
	// Host (address & port)
	regExp.setPattern( QWsServer::regExpHostStr );
	regExp.indexIn(request);
	QString host = regExp.cap(1);
	QStringList hostTmp = host.split(':');
	QString hostAddress = hostTmp[0];
	QString hostPort;
	if ( hostTmp.size() > 1 )
		hostPort = hostTmp[1];
	
	// Key
	QString key, key1, key2, key3;
	if ( version >= WS_V4 )
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
	
	////////////////////////////////////////////////////////////////////

	// If the mandatory fields are not specified, we abord the connection to the Websocket server
	if ( version == WS_VUnknow || resourceName.isEmpty() || hostAddress.isEmpty() || ( key.isEmpty() && ( key1.isEmpty() || key2.isEmpty() || key3.isEmpty() ) ) )
	{
		// Send bad request response
		QString response = QWsServer::composeBadRequestResponse( QList<EWebsocketVersion>() << WS_V6 << WS_V7 << WS_V8 << WS_V13 );
		clientSocket->write( response.toAscii() );
		clientSocket->flush();
		return;
	}
	
	////////////////////////////////////////////////////////////////////
	
	// Extract optional datas

	// Origin
	regExp.setPattern( QWsServer::regExpOriginStr );
	if ( regExp.indexIn(request) == -1 )
	{
		regExp.setPattern( QWsServer::regExpOrigin2Str );
		regExp.indexIn(request);
	}
	QString origin = regExp.cap(1);

	// Protocol
	regExp.setPattern( QWsServer::regExpProtocolStr );
	regExp.indexIn(request);
	QString protocol = regExp.cap(1);

	// Extensions
	regExp.setPattern( QWsServer::regExpExtensionsStr );
	regExp.indexIn(request);
	QString extensions = regExp.cap(1);
	
	////////////////////////////////////////////////////////////////////
	
	// Compose opening handshake response
	QString response;

	if ( version >= WS_V6 )
	{
		QString accept = computeAcceptV4( key );
		response = QWsServer::composeOpeningHandshakeResponseV6( accept, protocol );
	}
	else if ( version >= WS_V4 )
	{
		QString accept = computeAcceptV4( key );
		QString nonce = generateNonce();
		response = QWsServer::composeOpeningHandshakeResponseV4( accept, nonce, protocol );
	}
	else
	{
		QString accept = computeAcceptV0( key1, key2, key3 );
		response = QWsServer::composeOpeningHandshakeResponseV0( accept, origin, hostAddress, hostPort, resourceName , protocol );
	}
	
	// Handshake OK, disconnect readyRead
	disconnect( clientSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );

	// Send opening handshake response
	clientSocket->write( response.toAscii() );
	clientSocket->flush();

	// TEMPORARY CODE FOR LINUX COMPATIBILITY
	QWsSocket * wsSocket = new QWsSocket( clientSocket );
	wsSocket->setVersion( version );
	wsSocket->setResourceName( resourceName );
	wsSocket->setHost( host );
	wsSocket->setHostAddress( hostAddress );
	wsSocket->setHostPort( hostPort.toInt() );
	wsSocket->setOrigin( origin );
	wsSocket->setProtocol( protocol );
	wsSocket->setExtensions( extensions );
	addPendingConnection( wsSocket );
	emit newConnection();

	//connect( wsSocket, SIGNAL(closeAsked()), this, SLOT(closeSocket()) );

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

QString QWsServer::computeAcceptV0( QString key1, QString key2, QString key3 )
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

	int numSpaces1 = key1.count( ' ' );
	int numSpaces2 = key2.count( ' ' );

	num1 /= numSpaces1;
	num2 /= numSpaces2;

	QString concat = serializeInt( num1 ) + serializeInt( num2 ) + key3;

	QByteArray md5 = QCryptographicHash::hash( concat.toAscii(), QCryptographicHash::Md5 );
  
	return QString( md5 );
}

QString QWsServer::computeAcceptV4(QString key)
{
	key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	QByteArray hash = QCryptographicHash::hash ( key.toAscii(), QCryptographicHash::Sha1 );
	return hash.toBase64();
}

QString QWsServer::generateNonce()
{
	QByteArray nonce;
	int i = 16;
	while( i-- )
	{
		nonce.append( qrand() % 0x100 );
	}

	return QString( nonce.toBase64() );
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

QString QWsServer::composeOpeningHandshakeResponseV0( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol )
{
	QString response;
	
	response.append( "HTTP/1.1 101 WebSocket Protocol Handshake\r\n" );
	response.append( "Upgrade: Websocket\r\n" );
	response.append( "Connection: Upgrade\r\n" );
	response.append( "Sec-WebSocket-Origin: " + origin + "\r\n" );
	response.append( "Sec-WebSocket-Location: ws://" + hostAddress + ( hostPort.isEmpty() ? "" : (":"+hostPort) ) + resourceName + "\r\n" );
	if ( ! protocol.isEmpty() )
		response.append( "Sec-WebSocket-Protocol: " + protocol + "\r\n" );
	response.append( "\r\n" );
	response.append( accept );

	return response;
}

QString QWsServer::composeOpeningHandshakeResponseV4( QString accept, QString nonce, QString protocol )
{
	QString response;
	
	response.append( "HTTP/1.1 101 Switching Protocols\r\n" );
	response.append( "Upgrade: websocket\r\n" );
	response.append( "Connection: Upgrade\r\n" );
	response.append( "Sec-WebSocket-Accept: " + accept + "\r\n" );
	response.append( "Sec-WebSocket-Nonce: " + nonce + "\r\n" );
	if ( ! protocol.isEmpty() )
		response.append( "Sec-WebSocket-Protocol: " + protocol + "\r\n" );
	response.append( "\r\n" );

	return response;
}

QString QWsServer::composeOpeningHandshakeResponseV6( QString accept, QString protocol )
{
	QString response;
	
	response.append( "HTTP/1.1 101 Switching Protocols\r\n" );
	response.append( "Upgrade: websocket\r\n" );
	response.append( "Connection: Upgrade\r\n" );
	response.append( "Sec-WebSocket-Accept: " + accept + "\r\n" );
	if ( ! protocol.isEmpty() )
		response.append( "Sec-WebSocket-Protocol: " + protocol + "\r\n" );
	response.append( "\r\n" );

	return response;
}

QString QWsServer::composeBadRequestResponse( QList<EWebsocketVersion> versions )
{
	QString response;
	
	response.append( "HTTP/1.1 400 Bad Request\r\n" );
	if ( ! versions.isEmpty() )
	{
		QString versionsStr = QString::number( (int)versions.takeLast() );
		int i = versions.size();
		while ( i-- )
		{
			versionsStr.append( ", " + QString::number( (int)versions.takeLast() ) );
		}
		response.append( "Sec-WebSocket-Version: " + versionsStr + "\r\n" );
	}

	return response;
}