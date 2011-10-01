#include "QWsSocket.h"

int QWsSocket::maxBytesPerFrame = 125;

QWsSocket::QWsSocket(QObject * parent)
	: QTcpSocket(parent)
{
	setSocketState( QAbstractSocket::UnconnectedState );

	connect( this, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
	connect( this, SIGNAL(aboutToClose()), this, SLOT(aboutToClose()) );
}

QWsSocket::~QWsSocket()
{
}

void QWsSocket::dataReceived()
{
	currentFrame = QWsSocket::decodeFrame( this );
	emit frameReceived();
}

QByteArray QWsSocket::readFrame()
{
	QByteArray frameToReturn = currentFrame;
	currentFrame.clear();
	return frameToReturn;
}

qint64 QWsSocket::write ( const QByteArray & byteArray, int maxFrameBytes )
{
	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QList<QByteArray> framesList = QWsSocket::composeFrames( byteArray, maxFrameBytes );
	return writeFrames( framesList );
}

qint64 QWsSocket::writeFrames ( QList<QByteArray> framesList )
{
	qint64 nbBytesWritten = 0;
	for ( int i=0 ; i<framesList.size() ; i++ )
	{
		nbBytesWritten += QIODevice::write( framesList[i] );
	}
	return nbBytesWritten;
}

void QWsSocket::close( QString reason )
{
	// Compose and send close frame
	quint64 messageSize = reason.size();
	QByteArray maskingKey = generateMaskingKey();
	QByteArray BA;
	quint8 byte;

	QByteArray header = QWsSocket::composeHeader( true, OpClose, 0 );
	BA.append( header );

	// Reason // UNSUPPORTED FOR NOW
	
	QAbstractSocket::write( BA );

	QAbstractSocket::close();
}

void QWsSocket::aboutToClose()
{
	close( "Connection closed by QWsSocket" );
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

QByteArray QWsSocket::mask( QByteArray data, QByteArray maskingKey )
{
	for ( int i=0 ; i<data.size() ; i++ )
	{
		data[i] = ( data[i] ^ maskingKey[ i % 4 ] );
	}

	return data;
}

QByteArray QWsSocket::decodeFrame( QWsSocket * socket )
{
	QByteArray BA; // ReadBuffer
	quint8 byte; // currentByteBuffer

	// FIN, RSV1-3, Opcode
	BA = socket->read(1);
	byte = BA[0];
	quint8 FIN = (byte >> 7);
	quint8 RSV1 = ((byte & 0x7F) >> 6);
	quint8 RSV2 = ((byte & 0x3F) >> 5);
	quint8 RSV3 = ((byte & 0x1F) >> 4);
	quint8 Opcode = (byte & 0x0F);

	// Mask, PayloadLength
	BA = socket->read(1);
	byte = BA[0];
	quint8 Mask = (byte >> 7);
	quint8 PayloadLength = (byte & 0x7F);
	// Extended PayloadLength
	if ( PayloadLength == 126 )
	{
		BA = socket->read(2);
		//PayloadLength = (BA[0] + (BA[1] << 8));
		PayloadLength = BA.toUShort();
	}
	else if ( PayloadLength == 127 )
	{
		BA = socket->read(8);
		//PayloadLength = ((B[0] << 0*8) + (B[1] << 1*8) + (BA[2] << 2*8) + (BA[3] << 3*8) + (BA[4] << 4*8) + (BA[5] << 5*8) + (BA[6] << 6*8) + (BA[7] << 7*8));
		PayloadLength = BA.toULongLong();
	}

	// MaskingKey
	QByteArray MaskingKey;
	if ( Mask )
	{
		MaskingKey = socket->read(4);
	}

	// ExtensionData
	QByteArray ExtensionData;
	// Extension // UNSUPPORTED FOR NOW

	// ApplicationData
	QByteArray ApplicationData = socket->read( PayloadLength );
	if ( Mask )
		ApplicationData = QWsSocket::mask( ApplicationData, MaskingKey );

	return ApplicationData;
}

QList<QByteArray> QWsSocket::composeFrames( QByteArray byteArray, int maxFrameBytes )
{
	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QList<QByteArray> framesList;

	QByteArray maskingKey = generateMaskingKey();

	int nbFrames = byteArray.size() / maxFrameBytes + 1;

	for ( int i=0 ; i<nbFrames ; i++ )
	{
		QByteArray BA;

		// fin, size
		bool fin = true;
		quint64 size = byteArray.size();
		if ( i < nbFrames-1 ) // for multi-frames
		{
			fin = false;
			size = maxFrameBytes;
		}
		
		// Header
		QByteArray header = QWsSocket::composeHeader( fin, OpText, size, maskingKey );
		BA.append( header );
		
		// Application Data
		byteArray = QWsSocket::mask( byteArray, maskingKey );
		BA.append( byteArray );
		
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
	if ( payloadLength < 126 )
	{
		// FIN
		if ( fin )
			byte = (byte | 0x80);
		// Opcode
		byte = (byte | opcode);
	}
	else
	{
		 // UNSUPPORTED FOR NOW
	}
	// RSV1-3 // UNSUPPORTED FOR NOW
	BA.append( byte );

	// Mask, PayloadLength
	byte = 0x00;
	// Mask
	if ( maskingKey.size() == 4 )
		byte = (byte | 0x80);
	// PayloadLength
	if ( payloadLength < 126 )
	{
		byte = (byte | (quint8)payloadLength);
		BA.append( byte );
	}
	// Extended payloadLength
	else
	{
		QByteArray BAtmp;
		// 2 bytes
		if ( payloadLength <= 0xFFFF )
		{
			BAtmp.append( ( payloadLength >> 0*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 1*8 ) & 0xFF );
		}
		// 8 bytes
		else if ( payloadLength <= 0x7FFFFFFF )
		{
			BAtmp.append( ( payloadLength >> 0*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 1*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 2*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 3*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 4*8 ) & 0xFF );
			BAtmp.append( ( payloadLength >> 5*8 ) & 0xFF );
		}
		// bug on most significant bit
		else
		{
			// Frame cant be send in 1 frame // UNSUPPORTED FOR NOW
		}
		BA.append( BAtmp );
	}

	// Masking
	if ( maskingKey.size() == 4 )
		BA.append( maskingKey );

	return BA;
}
