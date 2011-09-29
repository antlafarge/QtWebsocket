#include "QWsSocket.h"

QWsSocket::QWsSocket(QObject * parent)
	: QAbstractSocket(QAbstractSocket::UnknownSocketType, parent)
{

}

QWsSocket::~QWsSocket()
{

}

QString QWsSocket::decodeFrame( QAbstractSocket * socket )
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
	// Extension data is ignored for now

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

QByteArray QWsSocket::composeFrame( QString message, int maxFrameBytes )
{
	quint64 messageSize = message.size();
	QByteArray BA;
	quint8 byte;

	// FIN, RSV1-3, Opcode
	byte = 0x00;
	if ( message.size() < 126 )
	{
		// FIN
		byte = (byte | 0x80);
		// Opcode
		byte = (byte | 0x01);
	}
	else
	{
		// FIN (depending of the FrameNum)
		// Opcode = 0
	}
	// RSV1-3
	// for now, do nothing
	BA.append( byte );

	// Mask, PayloadLength
	byte = 0x00;
	// Mask = 0
	// PayloadLength
	if ( messageSize < 126 )
	{
		quint8 sz = (quint8)messageSize;
		byte = (byte | sz);
		BA.append( byte );
	}
	// Extended payloadLength
	else
	{
		QByteArray BAtmp;
		// 2 bytes
		if ( messageSize <= 0xFFFF )
		{
			BAtmp = QByteArray::number( (quint16)messageSize );
		}
		// 8 bytes
		else if ( messageSize < 0xFFFF )
		{
			BAtmp = QByteArray::number( (quint64)messageSize );
		}
		// bug on most significant bit
		else
		{
			// BUG, STOP
			return QByteArray();
		}
		BA.append( BAtmp );
	}

	// Masking (ignored for now)

	// Application Data
	BA.append( message.toAscii() );

	return BA;
}
