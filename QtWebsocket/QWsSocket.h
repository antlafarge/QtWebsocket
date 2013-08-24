/*
Copyright (C) 2013 Antoine Lafarge qtwebsocket@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QRegExp>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTime>
#include <QStringList>

#include "QWsHandshake.h"

class QWsSocket : public QAbstractSocket
{
	Q_OBJECT

	friend class QWsServer;

public:
	enum EOpcode
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
	enum ECloseStatusCode
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

public:
	// ctor
	QWsSocket(QObject* parent = 0, QTcpSocket* socket = 0, EWebsocketVersion ws_v = WS_V13);
	// dtor
	virtual ~QWsSocket();

	// Public methods
	EWebsocketVersion version();
	QString resourceName();
	QString host();
	QHostAddress hostAddress();
	int hostPort();
	QString origin();
	QString protocol();
	QString extensions();

	void setResourceName(QString rn);
	void setHost(QString h);
	void setHostAddress(QString ha);
	void setHostPort(int hp);
	void setOrigin(QString o);
	void setProtocol(QString p);
	void setExtensions(QString e);

	qint64 write(const QString& string); // write data as text
	qint64 write(const QByteArray & byteArray); // write data as binary

public slots:
	void connectToHost(const QString & hostName, quint16 port = 80, OpenMode mode = ReadWrite);
	void connectToHost(const QHostAddress & address, quint16 port = 80, OpenMode mode = ReadWrite);
	void disconnectFromHost();
	void abort(QString reason = QString());
	void ping();

signals:
	void frameReceived(QString frame);
	void frameReceived(QByteArray frame);
	void pong(quint64 elapsedTime);

protected:
	qint64 writeFrames (const QList<QByteArray>& framesList);
	qint64 writeFrame (const QByteArray& byteArray);
	inline qint64 internalWrite(const QByteArray& string, bool asBinary);

protected slots:
	virtual void close(ECloseStatusCode closeStatusCode = NoCloseStatusCode, QString reason = QString());
	void processDataV0();
	void processDataV4();
	void processHandshake();
	void processTcpStateChanged(QAbstractSocket::SocketState socketState);
	void processTcpError(QAbstractSocket::SocketError err);

private:
	enum EReadingState
	{
		HeaderPending,
		PayloadLengthPending,
		BigPayloadLenghPending,
		MaskPending,
		PayloadBodyPending,
		CloseDataPending
	};

	// private vars
	QTcpSocket* tcpSocket;
	QByteArray currentFrame;
	QTime pingTimer;

	EWebsocketVersion _version;
	QString _resourceName;
	QString _host;
	QHostAddress _hostAddress;
	quint16 _hostPort;
	QString _origin;
	QString _protocol;
	QString _extensions;
	bool serverSideSocket;

	bool closingHandshakeSent;
	bool closingHandshakeReceived;

	EReadingState readingState;
	EOpcode opcode;
	bool isFinalFragment;
	bool hasMask;
	quint64 payloadLength;
	QByteArray maskingKey;
	ECloseStatusCode closeStatusCode;

	static const QString regExpAcceptStr;
	static const QString regExpUpgradeStr;
	static const QString regExpConnectionStr;
	static const QString connectionRefusedStr;
	QString handshakeResponse;
	QByteArray key;
	QByteArray key1;
	QByteArray key2;
	QByteArray key3;
	QByteArray accept;

public:
	// Static functions
	static QByteArray generateNonce();
	static QByteArray generateKey1or2();
	static QByteArray generateKey3();
	static QByteArray generateMaskingKey();
	static QByteArray generateMaskingKeyV4(QByteArray key, QByteArray nonce);
	static QByteArray computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray thirdPart);
	static QByteArray computeAcceptV4(QByteArray key);
	static QByteArray mask(const QByteArray & data, QByteArray & maskingKey);
	static QList<QByteArray> composeFrames(QByteArray byteArray, QByteArray& maskingKey, bool asBinary = false, int maxFrameBytes = 0);
	static QByteArray composeHeader(bool end, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray());
	static QString composeOpeningHandShakeV0(QString resourceName, QString host, QByteArray key1, QByteArray key2, QByteArray key3, QString origin = "", QString protocol = "", QString extensions = "");
	static QString composeOpeningHandShakeV13(QString resourceName, QString host, QByteArray key, QString origin = "", QString protocol = "", QString extensions = "");

	// static vars
	static const int maxBytesPerFrame = 1400;
	static const QLatin1String emptyLine;
	static QRegExp regExpIPv4;
	static QRegExp regExpHttpRequest;
	static QRegExp regExpHttpResponse;
	static QRegExp regExpHttpField;
};

#endif // QWSSOCKET_H
