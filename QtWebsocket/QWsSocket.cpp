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

#include "QWsSocket.h"

#include <QCryptographicHash>
#include <QtEndian>
#include <QHostInfo>
#include <QDataStream>
#include <QFile>

#include <iostream>

const QLatin1String QWsSocket::emptyLine("\r\n");
const QString QWsSocket::connectionRefusedStr(QLatin1String("Websocket connection refused"));

QRegExp QWsSocket::regExpIPv4(QLatin1String("^([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])(\\.([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){3}$"));
QRegExp QWsSocket::regExpHttpRequest(QLatin1String("^GET\\s(.*)\\sHTTP/(.+)\\r\\n"));
QRegExp QWsSocket::regExpHttpResponse(QLatin1String("^HTTP/1.1\\s(\\d{3})\\s(.+)\\r\\n"));
QRegExp QWsSocket::regExpHttpField(QLatin1String("^(.+):\\s(.+)\\r\\n$"));

QWsSocket::QWsSocket(QObject* parent, QTcpSocket* socket, EWebsocketVersion ws_v, bool useSsl2) :
	QAbstractSocket(QAbstractSocket::UnknownSocketType, parent),
	useSsl(useSsl2),
	tcpSocket(socket ? socket : (useSsl ? new QSslSocket : new QTcpSocket)),
	_version(ws_v),
	_hostPort(-1),
	serverSideSocket(false),
	closingHandshakeSent(false),
	closingHandshakeReceived(false),
	readingState(HeaderPending),
	isFinalFragment(false),
	hasMask(false),
	payloadLength(0),
	maskingKey(4, 0)
{
	tcpSocket->setParent(this);

	QAbstractSocket::setSocketState(tcpSocket->state());
	QAbstractSocket::setPeerAddress(tcpSocket->peerAddress());
	QAbstractSocket::setPeerPort(tcpSocket->peerPort());

	if (_version == WS_V0)
	{
		QObject::connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV0()));
	}
	else
	{
		QObject::connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV4()));
	}
	QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processTcpError(QAbstractSocket::SocketError)));
	QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
	QObject::connect(tcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
	QObject::connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processTcpStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(tcpSocket, SIGNAL(readChannelFinished()), this, SIGNAL(readChannelFinished()));
	QObject::connect(tcpSocket, SIGNAL(hostFound()), this, SIGNAL(hostFound()));
}

QWsSocket::~QWsSocket()
{
	QAbstractSocket::SocketState state = QAbstractSocket::state();
	if (state != QAbstractSocket::UnconnectedState)
	{
		close(CloseGoingAway, QLatin1String("The server destroyed the socket."));
		tcpSocket->abort();
		QAbstractSocket::setSocketState(QAbstractSocket::UnconnectedState);
		QAbstractSocket::stateChanged(QAbstractSocket::UnconnectedState);
		emit QAbstractSocket::disconnected();
	}
}

void QWsSocket::connectToHost(const QString& hostName, quint16 port, OpenMode mode)
{
	_hostPort = port;
	QString hostName2(hostName);
	hostName2 = hostName2.remove("ws://", Qt::CaseInsensitive);
	hostName2 = hostName2.remove("wss://", Qt::CaseInsensitive);
	if (hostName2.contains(QRegExp("^localhost$", Qt::CaseInsensitive)))
	{
		_host = QLatin1String("localhost");
		_hostAddress = QHostAddress::LocalHost;
	}
	else if (hostName2.contains(QRegExp(QWsSocket::regExpIPv4)))
	{
		_host = hostName2;
		_hostAddress = QHostAddress(hostName2);
	}
	else
	{
		_host = hostName2;
		QHostInfo info = QHostInfo::fromName(hostName2);
		QList<QHostAddress> hostAddresses = info.addresses();
		_hostAddress = hostAddresses[0];
	}

	if (useSsl)
	{
		QSslSocket* sslSocket = qobject_cast<QSslSocket*>(tcpSocket);
		
		QObject::connect(sslSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SIGNAL(sslErrors(const QList<QSslError>&)), Qt::UniqueConnection);
		QObject::connect(sslSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(displaySslErrors(const QList<QSslError>&)), Qt::UniqueConnection);

		QFile file("client-key.pem");
		if (!file.open(QIODevice::ReadOnly))
		{
			std::cout << "cant load client key" << std::endl;
			return;
		}
		QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray("qtwebsocket-client-key"));
		file.close();
		sslSocket->setPrivateKey(key);
		sslSocket->setLocalCertificate("client-crt.pem");
		if (!sslSocket->addCaCertificates("ca.pem"))
		{
			std::cout << "cant open ca certificate" << std::endl;
			return;
		}
		sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
		//sslSocket->ignoreSslErrors();
		QObject::connect(sslSocket, SIGNAL(encrypted()), this, SLOT(onEncrypted()), Qt::UniqueConnection);
		sslSocket->connectToHostEncrypted(hostName2, port);
		sslSocket->startClientEncryption();
	}
	else
	{
		QWsSocket::connectToHost(_hostAddress, _hostPort, mode);
	}
}

void QWsSocket::displaySslErrors(const QList<QSslError>& errors)
{
	for (int i=0, sz=errors.size(); i<sz; i++)
	{
		QString errorString = errors.at(i).errorString();
		std::cout << errorString.toStdString() << std::endl;
	}
}

void QWsSocket::connectToHost(const QHostAddress& address, quint16 port, OpenMode mode)
{
	if (useSsl)
	{
		QWsSocket::connectToHost(address.toString(), port, mode);
	}
	else
	{
		handshakeResponse.clear();
		setPeerAddress(address);
		setPeerPort(port);
		setOpenMode(mode);
		tcpSocket->connectToHost(address, port, mode);
	}
}

void QWsSocket::disconnectFromHost()
{
	QWsSocket::close();
}

void QWsSocket::abort(QString reason)
{
	QWsSocket::close(CloseAbnormalDisconnection, reason);
	tcpSocket->abort();
}

void QWsSocket::close(ECloseStatusCode closeStatusCode, QString reason)
{
	if (QAbstractSocket::state() == QAbstractSocket::UnconnectedState)
	{
		return;
	}

	if (! closingHandshakeSent)
	{
		switch (_version)
		{
			case WS_V4:
			case WS_V5:
			case WS_V6:
			case WS_V7:
			case WS_V8:
			case WS_V13:
			{
				// Compose and send close frame
				QByteArray BA;

				// Body
				if (closeStatusCode == NoCloseStatusCode)
				{
					// Header
					BA.append(QWsSocket::composeHeader(true, OpClose, 0));
				}
				else
				{
					// Header
					QByteArray maskingKey;
					if (! serverSideSocket)
						maskingKey = QWsSocket::generateMaskingKey();
					BA.append(QWsSocket::composeHeader(true, OpClose, reason.size() + 2, maskingKey));

					QByteArray body;

					// Close status code (optional)
					QByteArray ba_tmp;
					QDataStream ds(&ba_tmp, QIODevice::WriteOnly);
					ds << (quint16)closeStatusCode;
					body.append(ba_tmp);

					// Reason (optional)
					if (reason.size())
					{
						QByteArray reason_ba = reason.toLatin1();
						if (! serverSideSocket)
						{
							reason_ba = QWsSocket::mask(reason_ba, maskingKey);
						}
						body.append(reason_ba);
					}

					BA.append(body);
				}

				// Send closing handshake
				tcpSocket->write(BA);

				break;
			}
			case WS_V0:
			{
				QByteArray closeFrame;
				closeFrame.append((char)0xFF);
				closeFrame.append((char)0x00);
				tcpSocket->write(closeFrame);
				break;
			}
			default:
			{
				break;
			}
		}

		closingHandshakeSent = true;
	}

	if (QAbstractSocket::state() != QAbstractSocket::ClosingState)
	{
		QAbstractSocket::setSocketState(QAbstractSocket::ClosingState);
		emit QAbstractSocket::stateChanged(QAbstractSocket::ClosingState);
		emit QAbstractSocket::aboutToClose();
	}

	if (closingHandshakeSent && closingHandshakeReceived)
	{
		QAbstractSocket::setSocketState(QAbstractSocket::UnconnectedState);
		emit QAbstractSocket::stateChanged(QAbstractSocket::UnconnectedState);
		emit QAbstractSocket::disconnected();
		tcpSocket->disconnectFromHost();
	}
}

qint64 QWsSocket::write(const QString & string)
{
	if (_version == WS_V0)
	{
		return QWsSocket::write(string.toLatin1());
	}
	
	const QList<QByteArray>& framesList = QWsSocket::composeFrames(string.toLatin1(), false, maxBytesPerFrame);

	if(writeFrames(framesList) != -1)
	{
		emit bytesWritten(string.size());
		return string.size();
	}
	else
	{
		return -1;
	}
}

qint64 QWsSocket::write(const QByteArray & byteArray)
{
	if (_version == WS_V0)
	{
		QByteArray BA;
		BA.append((char)0x00);
		BA.append(byteArray);
		BA.append((char)0xFF);
		return writeFrame(BA);
	}

	const QList<QByteArray>& framesList = QWsSocket::composeFrames(byteArray, true, maxBytesPerFrame);

	if(writeFrames(framesList) != -1)
	{
		emit bytesWritten(byteArray.size());
		return byteArray.size();
	}
	else
	{
		return -1;
	}
}

void QWsSocket::processHandshake()
{
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket == NULL)
	{
		return;
	}
	
	QWsHandshake handshake(false);
	
	if (handshake.read(tcpSocket) == false)
	{
		emit error(QAbstractSocket::ConnectionRefusedError);
		return;
	}

	if (!handshake.readStarted || !handshake.complete)
	{
		return;
	}

	// If the mandatory params are not setted, we abord the connection to the Websocket server
	if (!handshake.isValid() || (QWsSocket::computeAcceptV4(key) != handshake.accept) )
	{
		emit error(QAbstractSocket::ConnectionRefusedError);
		return;
	}

	// handshake procedure succeeded
	QAbstractSocket::setSocketState(QAbstractSocket::ConnectedState);
	emit QAbstractSocket::stateChanged(QAbstractSocket::ConnectedState);
	emit QAbstractSocket::connected();
}

void QWsSocket::processDataV0()
{
	if(state() == QAbstractSocket::ConnectingState)
	{
		processHandshake();
		return;
	}

	QByteArray BA, buffer;
	quint8 type, b = 0x00;

	BA = tcpSocket->read(1); //TODO: refactor like processDataV4
	type = BA[0];

	if ((type & 0x80) == 0x00) // MSB of type not set
	{
		if (type != 0x00)
		{
			// ABORT CONNEXION
			tcpSocket->readAll();
			return;
		}
		
		// read data
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			if (b != 0xFF)
			{
				buffer.append(b);
			}
		} while (b != 0xFF);

		currentFrame.append(buffer);
	}
	else // MSB of type set
	{
		if (type != 0xFF)
		{
			// ERROR, ABORT CONNEXION
			close();
			return;
		}

		quint8 length = 0x00;
		
		bool bIsNotZero = true;
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			bIsNotZero = (b != 0x00 ? true : false);
			if (bIsNotZero) // b must be != 0
			{
				quint8 b_v = b & 0x7F;
				length *= 128;
				length += b_v;
			}
		} while (((b & 0x80) == 0x80) && bIsNotZero);

		BA = tcpSocket->read(length); // discard this bytes
	}

	if (currentFrame.size() > 0)
	{
		emit frameReceived(QString::fromLatin1(currentFrame));
		currentFrame.clear();
	}

	if (tcpSocket->bytesAvailable())
	{
		processDataV0();
	}
}

void QWsSocket::processDataV4()
{
	if(state() == QAbstractSocket::ConnectingState)
	{
		processHandshake();
	}
	else
		while (true)
			switch (readingState) {
				case HeaderPending: {
					if (tcpSocket->bytesAvailable() < 2)
						return;

					// FIN, RSV1-3, Opcode
					char header[2];
					tcpSocket->read(header, 2); // XXX: Handle return value
					isFinalFragment = (header[0] & 0x80) != 0;
					opcode = static_cast<EOpcode>(header[0] & 0x0F);

					// Mask, PayloadLength
					hasMask = (header[1] & 0x80) != 0;
					quint8 length = (header[1] & 0x7F);

					switch (length)
					{
						case 126:
							readingState = PayloadLengthPending;
							break;
						case 127:
							readingState = BigPayloadLenghPending;
							break;
						default:
							payloadLength = length;
							readingState = MaskPending;
							break;
					}
				}; break;
				case PayloadLengthPending: {
					if (tcpSocket->bytesAvailable() < 2)
						return;

					uchar length[2];
					tcpSocket->read(reinterpret_cast<char *>(length), 2); // XXX: Handle return value
					payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
					readingState = MaskPending;
				}; break;
				case BigPayloadLenghPending: {
					if (tcpSocket->bytesAvailable() < 8)
						return;

					uchar length[8];
					tcpSocket->read(reinterpret_cast<char *>(length), 8); // XXX: Handle return value
					// Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
					// XXX: Check for that?
					payloadLength = qFromBigEndian<quint64>(length) & ~(1LL << 63);
					readingState = MaskPending;
				}; break;
				case MaskPending: {
					if (!hasMask) {
						readingState = PayloadBodyPending;
						break;
					}

					if (tcpSocket->bytesAvailable() < 4)
						return;

					tcpSocket->read(maskingKey.data(), 4); // XXX: Handle return value

					if (opcode == OpClose)
					{
						readingState = CloseDataPending;
					}
					else
					{
						readingState = PayloadBodyPending;
					}
				}; /* Intentional fall-through */
				case PayloadBodyPending: {
					// TODO: Handle large payloads
					if (tcpSocket->bytesAvailable() < static_cast<qint32>(payloadLength))
					{
						return;
					}

					if (opcode == OpClose)
					{
						if (payloadLength >= 2 && tcpSocket->bytesAvailable() >= 2)
						{
							uchar bytes[2];
							tcpSocket->read(reinterpret_cast<char *>(bytes), 2);
							closeStatusCode = (ECloseStatusCode)qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(bytes));
						}
						else
						{
							closeStatusCode = NoCloseStatusCode;
						}
					}

					QByteArray ApplicationData = tcpSocket->read(payloadLength);
					if (hasMask)
					{
						ApplicationData = QWsSocket::mask(ApplicationData, maskingKey);
					}
					currentFrame.append(ApplicationData);

					readingState = HeaderPending;

					if (!isFinalFragment)
					{
						break;
					}

					switch (opcode)
					{
						case OpBinary:
							emit frameReceived(currentFrame);
							break;
						case OpText:
							emit frameReceived(QString::fromLatin1(currentFrame));
							break;
						case OpPing:
							write(QWsSocket::composeHeader(true, OpPong, 0));
							break;
						case OpPong:
							emit pong(pingTimer.elapsed());
							break;
						case OpClose:
							closingHandshakeReceived = true;
							close(closeStatusCode);
							break;
						default:
							// DO NOTHING
							break;
					}

					currentFrame.clear();
				}; break;
				case CloseDataPending:
				default:
					break;
			} /* while (true) switch */
}

qint64 QWsSocket::writeFrame(const QByteArray& byteArray)
{
	return tcpSocket->write(byteArray); // writes data to internal buffer and returns full size always; then emits signals
}

qint64 QWsSocket::writeFrames(const QList<QByteArray>& framesList)
{
	qint64 nbBytesWritten = 0;
	for (int i=0 ; i<framesList.size() ; i++)
	{
		nbBytesWritten += writeFrame(framesList[i]);
	}
	return nbBytesWritten;
}

void QWsSocket::onEncrypted()
{
	if (!serverSideSocket)
	{
		startHandshake();
	}
}

void QWsSocket::startHandshake()
{
	key = generateNonce();
	QString handshake = composeOpeningHandShake(QLatin1String("/"), _host, QLatin1String("QtWebsocket application"), key);
	tcpSocket->write(handshake.toLatin1());
}

void QWsSocket::processTcpStateChanged(QAbstractSocket::SocketState tcpSocketState)
{
	QAbstractSocket::SocketState wsSocketState = QAbstractSocket::state();
	switch (tcpSocketState)
	{
		case QAbstractSocket::HostLookupState:
		{
			QAbstractSocket::setSocketState(QAbstractSocket::HostLookupState);
			emit QAbstractSocket::stateChanged(QAbstractSocket::HostLookupState);
			break;
		}
		case QAbstractSocket::ConnectingState:
		{
			QAbstractSocket::setSocketState(QAbstractSocket::ConnectingState);
			emit QAbstractSocket::stateChanged(QAbstractSocket::ConnectingState);
			break;
		}
		case QAbstractSocket::ConnectedState:
		{
			if (!useSsl && wsSocketState == QAbstractSocket::ConnectingState)
			{
				setLocalAddress(tcpSocket->localAddress());
				setLocalPort(tcpSocket->localPort());
				startHandshake();
			}
			break;
		}
		case QAbstractSocket::ClosingState:
		{
			if (wsSocketState == QAbstractSocket::ConnectedState)
			{
				QWsSocket::close(CloseGoingAway);
				QAbstractSocket::setSocketState(QAbstractSocket::ClosingState);
				emit QAbstractSocket::stateChanged(QAbstractSocket::ClosingState);
				emit QAbstractSocket::aboutToClose();
			}
			break;
		}
		case QAbstractSocket::UnconnectedState:
		{
			if (wsSocketState != QAbstractSocket::UnconnectedState)
			{
				QAbstractSocket::setSocketError(QAbstractSocket::NetworkError);
				emit QAbstractSocket::error(QAbstractSocket::NetworkError);
				QAbstractSocket::setSocketState(QAbstractSocket::UnconnectedState);
				emit QAbstractSocket::stateChanged(QAbstractSocket::UnconnectedState);
				emit QAbstractSocket::disconnected();
			}
			closingHandshakeSent = false;
			closingHandshakeReceived = false;
			break;
		}
		default:
			break;
	}
}

QByteArray QWsSocket::generateNonce()
{
	qsrand(QDateTime::currentDateTime().toTime_t());

	QByteArray nonce;

	int i = 16;
	while(i--)
	{
		nonce.append(qrand() % 0x100);
	}

	return nonce.toBase64();
}

void QWsSocket::processTcpError(QAbstractSocket::SocketError err)
{
	setSocketError(tcpSocket->error());
	setErrorString(tcpSocket->errorString());
	emit error(err);
}

QByteArray QWsSocket::generateMaskingKey()
{
	QByteArray key;
	for (int i=0 ; i<4 ; i++)
	{
		key.append(qrand() % 0x100);
	}
	return key;
}

QByteArray QWsSocket::generateMaskingKeyV4(QString key, QString nonce)
{
	QString concat = key + nonce + QLatin1String("61AC5F19-FBBA-4540-B96F-6561F1AB40A8");
	QByteArray hash = QCryptographicHash::hash (concat.toLatin1(), QCryptographicHash::Sha1);
	return hash;
}

QByteArray QWsSocket::computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray key3)
{
	quint32 key_number_1 = QString::fromLatin1(key1).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();
	quint32 key_number_2 = QString::fromLatin1(key2).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();

	int spaces_1 = key1.count(' ');
	int spaces_2 = key2.count(' ');

	quint32 part_1 = key_number_1 / spaces_1;
	quint32 part_2 = key_number_2 / spaces_2;

	QByteArray challenge;
	QDataStream ds(&challenge, QIODevice::WriteOnly);
	ds << part_1 << part_2;
	challenge.append(key3);

	QByteArray md5 = QCryptographicHash::hash(challenge, QCryptographicHash::Md5);

	return md5;
}

QByteArray QWsSocket::computeAcceptV4(QByteArray key)
{
	key += QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	QByteArray hash = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
	return hash.toBase64();
}

QByteArray QWsSocket::mask(const QByteArray& data, QByteArray& maskingKey)
{
	QByteArray result;
	result.reserve(data.size());

	for (int i=0 ; i<data.size() ; i++)
	{
		result[i] = (data[i] ^ maskingKey[i % 4]);
	}

	return result;
}

QList<QByteArray> QWsSocket::composeFrames(QByteArray byteArray, bool asBinary, int maxFrameBytes)
{
	if (maxFrameBytes == 0)
	{
		maxFrameBytes = maxBytesPerFrame;
	}

	QList<QByteArray> framesList;

	QByteArray maskingKey;

	int nbFrames = byteArray.size() / maxFrameBytes + 1;

	for (int i=0 ; i<nbFrames ; i++)
	{
		QByteArray BA;

		// end, size
		bool end = false;
		quint64 size = maxFrameBytes;
		EOpcode opcode = OpContinue;
		if (i == nbFrames-1) // for multi-frames
		{
			end = true;
			size = byteArray.size();
		}
		if (i == 0)
		{
			if (asBinary)
				opcode = OpBinary;
			else
				opcode = OpText;
		}
		
		// Header
		BA.append(QWsSocket::composeHeader(end, opcode, size, maskingKey));
		
		// Application Data
		QByteArray dataForThisFrame = byteArray.left(size);
		byteArray.remove(0, size);
		
		//dataForThisFrame = QWsSocket::mask(dataForThisFrame, maskingKey);
		BA.append(dataForThisFrame);
		
		framesList << BA;
	}

	return framesList;
}

QByteArray QWsSocket::composeHeader(bool end, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey)
{
	QByteArray BA;
	quint8 byte;

	// end, RSV1-3, Opcode
	byte = 0x00;
	// end
	if (end)
	{
		byte = (byte | 0x80);
	}
	// Opcode
	byte = (byte | opcode);
	BA.append(byte);

	// Mask, PayloadLength
	byte = 0x00;
	QByteArray BAsize;
	// Mask
	if (maskingKey.size() == 4)
	{
		byte = (byte | 0x80);
	}
	// PayloadLength
	if (payloadLength <= 125)
	{
		byte = (byte | payloadLength);
	}
	// Extended payloadLength
	else
	{
		// 2 bytes
		if (payloadLength <= 0xFFFF)
		{
			byte = (byte | 126);
			BAsize.append((payloadLength >> 1*8) & 0xFF);
			BAsize.append((payloadLength >> 0*8) & 0xFF);
		}
		// 8 bytes
		else if (payloadLength <= 0x7FFFFFFF)
		{
			byte = (byte | 127);
			BAsize.append((payloadLength >> 7*8) & 0xFF);
			BAsize.append((payloadLength >> 6*8) & 0xFF);
			BAsize.append((payloadLength >> 5*8) & 0xFF);
			BAsize.append((payloadLength >> 4*8) & 0xFF);
			BAsize.append((payloadLength >> 3*8) & 0xFF);
			BAsize.append((payloadLength >> 2*8) & 0xFF);
			BAsize.append((payloadLength >> 1*8) & 0xFF);
			BAsize.append((payloadLength >> 0*8) & 0xFF);
		}
	}
	BA.append(byte);
	BA.append(BAsize);

	// Masking
	if (maskingKey.size() == 4)
	{
		BA.append(maskingKey);
	}

	return BA;
}

QString QWsSocket::composeOpeningHandShake(QString resourceName, QString host, QString origin, QByteArray key, QString protocol, QString extensions)
{
	QString hs;
	hs += QString("GET %1 HTTP/1.1\r\n").arg(resourceName);
	hs += QString("Host: %1\r\n").arg(host);
	hs += QLatin1String("Upgrade: websocket\r\n");
	hs += QLatin1String("Connection: Upgrade\r\n");
	hs += QString("Sec-WebSocket-Key: %1\r\n").arg(QLatin1String(key));
	hs += QString("Origin: %1\r\n").arg(origin);
	hs += QLatin1String("Sec-WebSocket-Version: 13\r\n");
	if (!protocol.isEmpty())
	{
		hs += QString("Sec-WebSocket-Protocol: %1\r\n").arg(protocol);
	}
	if (!extensions.isEmpty())
	{
		hs += QString("Sec-WebSocket-Extensions: %1\r\n").arg(extensions);
	}
	hs += QLatin1String("\r\n");
	return hs;
}

void QWsSocket::ping()
{
	pingTimer.restart();
	QByteArray pingFrame = QWsSocket::composeHeader(true, OpPing, 0);
	writeFrame(pingFrame);
}

void QWsSocket::setResourceName(QString rn)
{
	_resourceName = rn;
}

void QWsSocket::setHost(QString h)
{
	_host = h;
}

void QWsSocket::setHostAddress(QString ha)
{
	_hostAddress = ha;
}

void QWsSocket::setHostPort(int hp)
{
	_hostPort = hp;
}

void QWsSocket::setOrigin(QString o)
{
	_origin = o;
}

void QWsSocket::setProtocol(QString p)
{
	_protocol = p;
}

void QWsSocket::setExtensions(QString e)
{
	_extensions = e;
}

EWebsocketVersion QWsSocket::version()
{
	return _version;
}

QString QWsSocket::resourceName()
{
	return _resourceName;
}

QString QWsSocket::host()
{
	return _host;
}

QHostAddress QWsSocket::hostAddress()
{
	return _hostAddress;
}

int QWsSocket::hostPort()
{
	return _hostPort;
}

QString QWsSocket::origin()
{
	return _origin;
}

QString QWsSocket::protocol()
{
	return _protocol;
}

QString QWsSocket::extensions()
{
	return _extensions;
}
