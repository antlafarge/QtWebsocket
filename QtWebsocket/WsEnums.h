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

#ifndef WSENUMS_H
#define WSENUMS_H

namespace QtWebsocket
{
	
enum Protocol
{
	None = 0x0,
	Tcp = 0x1,
	Tls = 0x2
	//TcpTls= 0x3 // soon
};

enum WsMode
{
	WsClientMode = 1,
	WsServerMode = 2
};

enum Opcode
{
	OpContinue = 0x0,
	OpText = 0x1,
	OpBinary = 0x2,
	OpReserved3 = 0x3,
	OpReserved4 = 0x4,
	OpReserved5 = 0x5,
	OpReserved6 = 0x6,
	OpReserved7 = 0x7,
	OpClose = 0x8,
	OpPing = 0x9,
	OpPong = 0xA,
	OpReservedB = 0xB,
	OpReservedV = 0xC,
	OpReservedD = 0xD,
	OpReservedE = 0xE,
	OpReservedF = 0xF
};

enum CloseStatusCode
{
	NoCloseStatusCode = 0,
	CloseNormal = 1000,
	CloseGoingAway = 1001,
	CloseProtocolError = 1002,
	CloseDataTypeNotSupported = 1003,
	CloseReserved1004 = 1004,
	CloseMissingStatusCode = 1005,
	CloseAbnormalDisconnection = 1006,
	CloseWrongDataType = 1007,
	ClosePolicyViolated = 1008,
	CloseTooMuchData = 1009,
	CloseMissingExtension = 1010,
	CloseBadOperation = 1011,
	CloseTLSHandshakeFailed = 1015
};

enum ReadingState
{
	HeaderPending,
	PayloadLengthPending,
	BigPayloadLenghPending,
	MaskPending,
	PayloadBodyPending,
	CloseDataPending
};

} // namespace QtWebsocket

#endif // WSENUMS_H
