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

#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QRegExp>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSsl>
#include <QSslKey>
#include <QHostAddress>
#include <QTime>
#include <QStringList>

#include "WsEnums.h"
#include "QWsHandshake.h"
#include "QWsFrame.h"

namespace QtWebsocket
{

class QWsSocket : public QAbstractSocket
{
	Q_OBJECT

	friend class QWsServer;

public:
	// ctor
	QWsSocket(QObject* parent = NULL, QTcpSocket* socket = NULL, EWebsocketVersion ws_v = WS_V13);
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
	void encrypted();
	void sslErrors(const QList<QSslError>& errors);

protected:
	qint64 writeFrames (const QList<QByteArray>& framesList);
	qint64 writeFrame (const QByteArray& byteArray);
	inline qint64 internalWrite(const QByteArray& string, bool asBinary);
	void initTcpSocket();

protected slots:
	virtual void close(CloseStatusCode closeStatusCode = NoCloseStatusCode, QString reason = QString());
	void processDataV0();
	void processDataV4();
	void processHandshake();
	void processTcpStateChanged(QAbstractSocket::SocketState socketState);
	void processTcpError(QAbstractSocket::SocketError err);
	void startHandshake();
	void onEncrypted();

private:

	// private vars
	QTcpSocket* tcpSocket;
	QByteArray currentFrame;
	QTime pingTimer;
	
	WsMode _wsMode;

	QWsFrame* _currentFrame;
	QByteArray currentData;
	Opcode currentDataOpcode;

	/*!
	 * True if we are waiting for a final data fragment.
	 */
	bool continuation;

	EWebsocketVersion _version;
	QString _resourceName;
	QString _hostName;
	QString _host;
	QHostAddress _hostAddress;
	quint16 _hostPort;
	QString _origin;
	QString _protocol;
	QString _extensions;

	bool closingHandshakeSent;
	bool closingHandshakeReceived;

	Opcode currentOpcode;
	CloseStatusCode closeStatusCode;

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

	bool _secured;

	/*!
	 * Sends pong response with `applicationData` appended.
	 */
	void handlePing(QByteArray applicationData = QByteArray());

	/*!
	 * Processes the joined payload of the previous frames.
	 *
	 * Called if the final and valid non-control frame has been received.
	 */
	void handleData();

	/*!
	 * Processes the current control frame.
	 *
	 * Responds to ping and pong or closes connection according to
	 * `currentOpcode`. Called if a complete and valid control frame has been
	 * received.
	 */
	void handleControlFrame();

public:
	// Static functions
	static QByteArray generateNonce();
	static QByteArray generateKey1or2();
	static QByteArray generateKey3();
	static QByteArray generateMaskingKey();
	static QByteArray generateMaskingKeyV4(QByteArray key, QByteArray nonce);
	static QByteArray computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray thirdPart);
	static QByteArray computeAcceptV4(QByteArray key);
	static QByteArray mask(const QByteArray& data, QByteArray& maskingKey);
	static QList<QByteArray> composeFrames(QByteArray data, Opcode opcode = OpText, QByteArray maskingKey = QByteArray(), int maxFrameBytes = 0);
	static QByteArray composeHeader(bool end, Opcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray());
	static QString composeOpeningHandShakeV0(QString resourceName, QString host, QByteArray key1, QByteArray key2, QByteArray key3, QString origin = "", QString protocol = "", QString extensions = "");
	static QString composeOpeningHandShakeV13(QString resourceName, QString host, QByteArray key, QString origin = "", QString protocol = "", QString extensions = "");

	/*!
	 * Encapsulates `applicationData` in a single Frame.
	 *
	 * The frame size is deduced from `applicationData.size()` and there are no
	 * checks performed on it.
	 */
	static QByteArray composeFrame(QByteArray applicationData, Opcode opcode, bool final = true);

	// static vars
	static const int maxBytesPerFrame = 1400;
	static const QLatin1String emptyLine;
	static QRegExp regExpIPv4;
	static QRegExp regExpHttpRequest;
	static QRegExp regExpHttpResponse;
	static QRegExp regExpHttpField;
};

} // namespace QtWebsocket

#endif // QWSSOCKET_H
