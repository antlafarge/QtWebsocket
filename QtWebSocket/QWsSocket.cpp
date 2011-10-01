#include "QWsSocket.h"

QWsSocket::QWsSocket(QObject * parent)
	: QTcpSocket(parent)
{
	setSocketState( QAbstractSocket::UnconnectedState );
}

QWsSocket::~QWsSocket()
{
}

QString QWsSocket::readFrame()
{
	return QWsSocket::decodeFrame( this );
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

QString QWsSocket::decodeFrame( QTcpSocket * socket )
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

	return QString( ApplicationData );
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
	// Mask = 0
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

	// Masking // UNSUPPORTED FOR NOW

	// Application Data
	BA.append( byteArray );

	return BA;
}
