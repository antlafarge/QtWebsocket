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

#ifndef QWSHANDSHAKE_H
#define QWSHANDSHAKE_H

#include <QTcpSocket>

#include "WsEnums.h"

namespace QtWebsocket
{

enum EWebsocketVersion
{
	WS_VUnknow = -1,
	WS_V0 = 0,
	WS_V4 = 4,
	WS_V5 = 5,
	WS_V6 = 6,
	WS_V7 = 7,
	WS_V8 = 8,
	WS_V13 = 13
};

class QWsHandshake
{
public:
	QWsHandshake(WsMode wsMode = WsClientMode);
	~QWsHandshake();

	bool read(QTcpSocket* tcpSocket);
	bool isValid();
	bool isValidCommonPart();
	bool isValidServerPart();
	bool isValidClientPart();

	WsMode _wsMode;

	bool readStarted;
	bool complete;
	
	QHash<QString, QString> fields;

	QString rawHandshake;
	
	QString httpRequestVersion;
	bool httpRequestValid;
	QString errorString;

	QString resourceName;
	
	QString host;
	QString hostAddress;
	QString hostPort;

	EWebsocketVersion version;
	QByteArray key;
	QByteArray key1;
	QByteArray key2;
	QByteArray key3;

	QByteArray accept;

	QString origin;
	QString protocol;
	QString extensions;
};

} // namespace QtWebsocket

#endif // QWSHANDSHAKE_H
