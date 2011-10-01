#include "QWsSocket.h"

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

qint64	QWsSocket::write ( const QByteArray & byteArray )
{
	QAbstractSocket::write( QWsSocket::composeFrame( byteArray ) );
	return byteArray.size(); // IMPROVE LATER
}

void QWsSocket::close( QString reason )
{
	// Compose and send close frame
	quint64 messageSize = reason.size();
	QByteArray BA;
	quint8 byte;

	// FIN, RSV1-3, Opcode
	byte = 0x00;
	// FIN
	byte = (byte | 0x80);
	// Opcode
	byte = (byte | 0x08);
	// RSV1-3 // UNSUPPORTED FOR NOW
	BA.append( byte );

	// Mask, PayloadLength
	byte = 0x00;
	// Mask // UNSUPPORTED FOR NOW
	// PayloadLength // UNSUPPORTED FOR NOW
	BA.append( byte );
	// Extended payloadLength // UNSUPPORTED FOR NOW

	// Masking // UNSUPPORTED FOR NOW

	// Reason // UNSUPPORTED FOR NOW
	
	QAbstractSocket::write( BA );

	QAbstractSocket::close();
}

void QWsSocket::aboutToClose()
{
	close( "connection closed 1337" );
}

QByteArray QWsSocket::generateRandomMask()
{
	QByteArray key;
	for ( int i=0 ; i<4 ; i++ )
	{
		key.append( qrand() % 0x100 );
	}

	return key;
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
	{
		for ( int i=0 ; i<PayloadLength ; i++ )
		{
			ApplicationData[i] = ( ApplicationData[i] ^ MaskingKey[ i % 4 ] );
		}
	}

	return ApplicationData;
}

QByteArray QWsSocket::composeFrame( QByteArray byteArray, int maxFrameBytes )
{
	quint64 size = byteArray.size();
	QByteArray BA;
	quint8 byte;

	// FIN, RSV1-3, Opcode
	byte = 0x00;
	if ( size < 126 )
	{
		// FIN
		byte = (byte | 0x80);
		// Opcode
		byte = (byte | 0x01);
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
	byte = (byte | 0x80);
	// PayloadLength
	if ( size < 126 )
	{
		quint8 sz = (quint8)size;
		byte = (byte | sz);
		BA.append( byte );
	}
	// Extended payloadLength
	else
	{
		QByteArray BAtmp;
		// 2 bytes
		if ( size <= 0xFFFF )
		{
			BAtmp = QByteArray::number( (quint16)size );
		}
		// 8 bytes
		else if ( size < 0xFFFF )
		{
			BAtmp = QByteArray::number( (quint64)size );
		}
		// bug on most significant bit
		else
		{
			// Frame cant be send in 1 frame // UNSUPPORTED FOR NOW
		}
		BA.append( BAtmp );
	}

	// Masking
	QByteArray maskingKey = generateRandomMask();
	BA.append( maskingKey );

	// ApplicationData
	for ( int i=0 ; i<size ; i++ )
	{
		byteArray[i] = ( byteArray[i] ^ maskingKey[ i % 4 ] );
	}
	BA.append( byteArray );

	return BA;
}
