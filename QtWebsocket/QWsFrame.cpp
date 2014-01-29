/*
Copyright 2013 Antoine Lafarge qtwebsocket@gmail.com

This file is part of QtWebsocket.

QtWebsocket is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

QtWebsocket is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QtWebsocket.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "QWsFrame.h"
#include "QWsSocket.h"

namespace QtWebsocket
{
	
QWsFrame::QWsFrame() :
	readingState(HeaderPending),
	final(false),
	rsv(0),
	hasMask(false),
	payloadLength(0)
{}


void QWsFrame::clear()
{
	payload.clear();
	readingState = HeaderPending;
}


QByteArray QWsFrame::data() const
{
	QByteArray result;
	result.reserve(payload.size());
	if (hasMask) {
		for (int i=0 ; i<payload.size() ; i++)
			result[i] = (payload[i] ^ maskingKey[i % 4]);
		return result;
	}
	else
		return payload;
}

bool QWsFrame::controlFrame() const
{
	return opcode >= 0x8;
}

// TODO implement, finished flag;
bool QWsFrame::valid() const
{
	if (payloadLength >> 63) // Most significant bit must be 0
		return false;
	if (rsv & 0x70)
		return false;
	if (opcode >= 0x3 && opcode <= 0x7) // Reserved opcode
		return false;
	if (opcode >= 0xB && opcode <= 0xF) // Reserved control opcode
		return false;
	if (controlFrame() && !final) // Control frames must not be fragmented
		return false;
	if (controlFrame() && payloadLength > 125) // Control frames must have small payload
		return false;

	return true;
}

} // namespace QtWebsocket
