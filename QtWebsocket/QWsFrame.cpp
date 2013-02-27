#include "QWsFrame.h"

QWsFrame::QWsFrame() :
	readingState( QWsSocket::HeaderPending ),
	hasMask( false ),
	final( false ),
	payloadLength( 0 ),
	rsv( 0 )
{}


void QWsFrame::clear()
{
	payload.clear();
	readingState = QWsSocket::HeaderPending;
}


QByteArray QWsFrame::data() const
{
	QByteArray result;
	result.reserve( payload.size() );
	if ( hasMask ) {
		for ( int i=0 ; i<payload.size() ; i++ )
			result[i] = ( payload[i] ^ maskingKey[ i % 4 ] );
		return result;
	}
	else
		return payload;
}


// TODO implement, finished flag;
bool QWsFrame::valid()
{
	if ( payloadLength >> 63 ) // Most significant bit must be 0
		return false;
	if ( rsv & 0x70 )
		return false;
	if ( opcode >= 0x3 && opcode <= 0x7 ) // Reserved opcode
		return false;
	if ( opcode >= 0xB && opcode <= 0xF ) // Reserved control opcode
		return false;
	if ( opcode >= 0x3 && !final )
		return false;

	return true;
}
