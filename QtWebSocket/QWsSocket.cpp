#include "QWsSocket.h"

#include <QCryptographicHash>
#include <QtEndian>

int QWsSocket::maxBytesPerFrame = 1400;

QWsSocket::QWsSocket( QTcpSocket * socket, QObject * parent ) :
	QAbstractSocket( QAbstractSocket::UnknownSocketType, parent ),
	state(HeaderPending),
	isFinalFragment(false),
	hasMask(false),
	payloadLength(0),
	maskingKey(4, 0)
{
	tcpSocket = socket;
	_version = WS_VUnknow;
	_hostPort = -1;

	//setSocketState( QAbstractSocket::UnconnectedState );
	setSocketState( socket->state() );

	connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
	// XXX: Consider connecting socket's signals to own signals directly
	connect( tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()) );
	connect( tcpSocket, SIGNAL(aboutToClose()), this, SLOT(tcpSocketAboutToClose()) );
}

QWsSocket::~QWsSocket()
{
}

void QWsSocket::dataReceived()
{
	if ( _version == WS_V0 )
	{
		dataReceivedV0();
		return;
	}

	while (true) switch (state) {
	case HeaderPending: {
		if (tcpSocket->bytesAvailable() < 2)
			return;

		// FIN, RSV1-3, Opcode
		char header[2];
		tcpSocket->read(header, 2); // XXX: Handle return value
		isFinalFragment = (header[0] & 0x80) != 0;
		opcode = static_cast<EOpcode>(header[0] & 0x0F);

		// Mask, PayloadLength
		hasMask = (header[1] & 0x80) != 0;
		quint8 length = (header[1] & 0x7F);

		switch (length) {
		case 126:
			state = PayloadLengthPending;
			break;
		case 127:
			state = BigPayloadLenghPending;
			break;
		default:
			payloadLength = length;
			state = MaskPending;
			break;
		}
	}; break;
	case PayloadLengthPending: {
		if (tcpSocket->bytesAvailable() < 2)
			return;

		uchar length[2];
		tcpSocket->read(reinterpret_cast<char *>(length), 2); // XXX: Handle return value
		payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
		state = MaskPending;
	}; break;
	case BigPayloadLenghPending: {
		if (tcpSocket->bytesAvailable() < 8)
			return;

		uchar length[8];
		tcpSocket->read(reinterpret_cast<char *>(length), 8); // XXX: Handle return value
		// Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
		// XXX: Check for that?
		payloadLength = qFromBigEndian<quint64>(length) & ~(1L << 63);
		state = MaskPending;
	}; break;
	case MaskPending: {
		if (!hasMask) {
			state = PayloadBodyPending;
			break;
		}

		if (tcpSocket->bytesAvailable() < 4)
			return;

		tcpSocket->read(maskingKey.data(), 4); // XXX: Handle return value
		state = PayloadBodyPending;
	}; /* Intentional fall-through */
	case PayloadBodyPending: {
		// TODO: Handle large payloads
		if (tcpSocket->bytesAvailable() < static_cast<qint32>(payloadLength))
			return;

		// Extension // UNSUPPORTED FOR NOW
		QByteArray ApplicationData = tcpSocket->read( payloadLength );
		if ( hasMask )
			ApplicationData = QWsSocket::mask( ApplicationData, maskingKey );
		currentFrame.append( ApplicationData );

		state = HeaderPending;

		if ( !isFinalFragment )
			break;

		switch ( opcode )
		{
			case OpBinary:
				emit frameReceived( currentFrame );
				break;
			case OpText:
				emit frameReceived( QString::fromAscii(currentFrame) );
				break;
			case OpPing:
				write( QWsSocket::composeHeader( true, OpPong, 0 ) );
				break;
			case OpPong:
				emit pong( pingTimer.elapsed() );
				break;
			case OpClose:
				tcpSocket->close();
				break;
		}

		currentFrame.clear();
	}; break;
	} /* while (true) switch */
}

void QWsSocket::dataReceivedV0()
{
	QByteArray BA, rawData;
	quint8 type, b = 0x00;

	BA = tcpSocket->read(1);
	type = BA[0];

	if ( ( type & 0x80 ) == 0x00 ) // MSB of type not set
	{
		if ( type != 0x00 )
		{
			// ABORT CONNEXION
			tcpSocket->readAll();
			return;
		}
		
		// read data
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			if ( b != 0xFF )
				rawData.append( b );
		} while ( b != 0xFF );

		currentFrame.append( rawData );
	}
	else // MSB of type set
	{
		if ( type != 0xFF )
		{
			// ABORT CONNEXION
			tcpSocket->readAll();
			return;
		}

		quint8 length = 0x00;
		
		bool bIsNotZero = true;
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			bIsNotZero = ( b != 0x00 ? true : false );
			if ( bIsNotZero ) // b must be != 0
			{
				quint8 b_v = b & 0x7F;
				length *= 128;
				length += b_v;
			}
		} while ( ( ( b & 0x80 ) == 0x80 ) && bIsNotZero );

		BA = tcpSocket->read(length); // discard this bytes
	}

	if ( currentFrame.size() > 0 )
	{
		QString byteString;
		byteString.reserve( currentFrame.size() );
		for (int i=0 ; i<currentFrame.size() ; i++)
			byteString[i] = currentFrame[i];
		emit frameReceived( byteString );
		currentFrame.clear();
	}

	if ( tcpSocket->bytesAvailable() )
		dataReceived();
}

qint64 QWsSocket::write ( const QString & string, int maxFrameBytes )
{
	if ( _version == WS_V0 )
	{
		return QWsSocket::write( string.toAscii(), maxFrameBytes );
	}

	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QList<QByteArray> framesList = QWsSocket::composeFrames( string.toAscii(), false, maxFrameBytes );
	return writeFrames( framesList );
}

qint64 QWsSocket::write ( const QByteArray & byteArray, int maxFrameBytes )
{
	if ( _version == WS_V0 )
	{
		QByteArray BA;
		BA.append( (char)0x00 );
		BA.append( byteArray );
		BA.append( (char)0xFF );
		return writeFrame( BA );
	}

	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QList<QByteArray> framesList = QWsSocket::composeFrames( byteArray, true, maxFrameBytes );
	return writeFrames( framesList );
}

qint64 QWsSocket::writeFrame ( const QByteArray & byteArray )
{
	return tcpSocket->write( byteArray );
}

qint64 QWsSocket::writeFrames ( QList<QByteArray> framesList )
{
	qint64 nbBytesWritten = 0;
	for ( int i=0 ; i<framesList.size() ; i++ )
	{
		nbBytesWritten += writeFrame( framesList[i] );
	}
	return nbBytesWritten;
}

void QWsSocket::close( QString reason )
{
	if ( _version == WS_V0 )
	{
		QByteArray BA;
		BA.append( (char)0xFF );
		BA.append( (char)0x00 );
		tcpSocket->write(BA);
		tcpSocket->close();
		return;
	}

	// Compose and send close frame
	quint64 messageSize = reason.size();
	QByteArray BA;
	quint8 byte;

	QByteArray header = QWsSocket::composeHeader( true, OpClose, 0 );
	BA.append( header );

	// Reason // UNSUPPORTED FOR NOW
	
	tcpSocket->write( BA );

	tcpSocket->close();
}

void QWsSocket::tcpSocketAboutToClose()
{
	emit aboutToClose();
}

void QWsSocket::tcpSocketDisconnected()
{
	emit disconnected();
}

QByteArray QWsSocket::generateMaskingKey()
{
	QByteArray key;
	for ( int i=0 ; i<4 ; i++ )
	{
		key.append( qrand() % 0x100 );
	}

	return key;
}

QByteArray QWsSocket::generateMaskingKeyV4( QString key, QString nonce )
{
	QString concat = key + nonce + "61AC5F19-FBBA-4540-B96F-6561F1AB40A8";
	QByteArray hash = QCryptographicHash::hash ( concat.toAscii(), QCryptographicHash::Sha1 );
	return hash;
}

QByteArray QWsSocket::mask( QByteArray data, QByteArray maskingKey )
{
	for ( int i=0 ; i<data.size() ; i++ )
	{
		data[i] = ( data[i] ^ maskingKey[ i % 4 ] );
	}

	return data;
}

QList<QByteArray> QWsSocket::composeFrames( QByteArray byteArray, bool asBinary, int maxFrameBytes )
{
	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QList<QByteArray> framesList;

	QByteArray maskingKey;// = generateMaskingKey();

	int nbFrames = byteArray.size() / maxFrameBytes + 1;

	for ( int i=0 ; i<nbFrames ; i++ )
	{
		QByteArray BA;

		// fin, size
		bool fin = false;
		quint64 size = maxFrameBytes;
		EOpcode opcode = OpContinue;
		if ( i == nbFrames-1 ) // for multi-frames
		{
			fin = true;
			size = byteArray.size();
		}
		if ( i == 0 )
		{
			if ( asBinary )
				opcode = OpBinary;
			else
				opcode = OpText;
		}
		
		// Header
		QByteArray header = QWsSocket::composeHeader( fin, opcode, size, maskingKey );
		BA.append( header );
		
		// Application Data
		QByteArray dataForThisFrame = byteArray.left( size );
		byteArray.remove( 0, size );
		
		//dataForThisFrame = QWsSocket::mask( dataForThisFrame, maskingKey );
		BA.append( dataForThisFrame );
		
		framesList << BA;
	}

	return framesList;
}

QByteArray QWsSocket::composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey )
{
	QByteArray BA;
	quint8 byte;

	// FIN, RSV1-3, Opcode
	byte = 0x00;
	// FIN
	if ( fin )
		byte = (byte | 0x80);
	// Opcode
	byte = (byte | opcode);
	BA.append( byte );

	// Mask, PayloadLength
	byte = 0x00;
	QByteArray BAsize;
	// Mask
	if ( maskingKey.size() == 4 )
		byte = (byte | 0x80);
	// PayloadLength
	if ( payloadLength <= 125 )
	{
		byte = (byte | payloadLength);
	}
	// Extended payloadLength
	else
	{
		// 2 bytes
		if ( payloadLength <= 0xFFFF )
		{
			byte = ( byte | 126 );
			BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
		}
		// 8 bytes
		else if ( payloadLength <= 0x7FFFFFFF )
		{
			byte = ( byte | 127 );
			BAsize.append( ( payloadLength >> 7*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 6*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 5*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 4*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 3*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 2*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
		}
	}
	BA.append( byte );
	BA.append( BAsize );

	// Masking
	if ( maskingKey.size() == 4 )
		BA.append( maskingKey );

	return BA;
}

void QWsSocket::ping()
{
	pingTimer.restart();
	QByteArray pingFrame = QWsSocket::composeHeader( true, OpPing, 0 );
	writeFrame( pingFrame );
}

void QWsSocket::setVersion( EWebsocketVersion ws_v )
{
	_version = ws_v;
}

void QWsSocket::setResourceName( QString rn )
{
	_resourceName = rn;
}

void QWsSocket::setHost( QString h )
{
	_host = h;
}

void QWsSocket::setHostAddress( QString ha )
{
	_hostAddress = ha;
}

void QWsSocket::setHostPort( int hp )
{
	_hostPort = hp;
}

void QWsSocket::setOrigin( QString o )
{
	_origin = o;
}

void QWsSocket::setProtocol( QString p )
{
	_protocol = p;
}

void QWsSocket::setExtensions( QString e )
{
	_extensions = e;
}

EWebsocketVersion QWsSocket::version()
{
	return _version;
}

QString QWsSocket::resourceName()
{
	return _resourceName;
}

QString QWsSocket::host()
{
	return _host;
}

QString QWsSocket::hostAddress()
{
	return _hostAddress;
}

int QWsSocket::hostPort()
{
	return _hostPort;
}

QString QWsSocket::origin()
{
	return _origin;
}

QString QWsSocket::protocol()
{
	return _protocol;
}

QString QWsSocket::extensions()
{
	return _extensions;
}

QString QWsSocket::composeOpeningHandShake( QString resourceName, QString host, QString origin, QString extensions, QString key )
{
	QString hs;
	hs.append("GET " + resourceName + " HTTP/1.1\r\n");
	hs.append("Host: " + host + "\r\n");
	hs.append("Upgrade: websocket\r\n");
	hs.append("Connection: Upgrade\r\n");
	hs.append("Sec-WebSocket-Key: " + key + "\r\n");
	hs.append("Origin: " + origin + "\r\n");
	hs.append("Sec-WebSocket-Extensions: " + extensions + "\r\n");
	hs.append("Sec-WebSocket-Version: 13\r\n");
	hs.append("\r\n");
	return hs;
}
