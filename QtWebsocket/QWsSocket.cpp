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

#include "QWsSocket.h"

#include <QCryptographicHash>
#include <QtEndian>
#include <QHostInfo>
#include <QDataStream>
#include <QFile>
#include <QtCore/qmath.h>

#include <iostream>

#include "QWsServer.h"
#include "QWsFrame.h"
#include "functions.h"

namespace QtWebsocket
{

const QLatin1String QWsSocket::emptyLine("\r\n");
const QString QWsSocket::connectionRefusedStr(QLatin1String("Websocket connection refused"));
const QRegExp regExpUriStart("^wss?://", Qt::CaseInsensitive);
const QRegExp regExpUri("^wss?://([^\\s]+)$", Qt::CaseInsensitive);
const QRegExp regExpLocalhostUri("^localhost$", Qt::CaseInsensitive);

QRegExp QWsSocket::regExpIPv4(QLatin1String("^([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])(\\.([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){3}$"));
QRegExp QWsSocket::regExpHttpRequest(QLatin1String("^GET\\s(.*)\\sHTTP/(.+)\\r\\n"));
QRegExp QWsSocket::regExpHttpResponse(QLatin1String("^HTTP/1.1\\s(\\d{3})\\s(.+)\\r\\n"));
QRegExp QWsSocket::regExpHttpField(QLatin1String("^(.+):\\s(.+)\\r\\n$"));

QWsSocket::QWsSocket(QObject* parent, QTcpSocket* socket, EWebsocketVersion ws_v) :
	QAbstractSocket(QAbstractSocket::UnknownSocketType, parent),
	tcpSocket(socket ? socket : new QTcpSocket),
	_wsMode(WsClientMode),
	_currentFrame(new QWsFrame),
	continuation(false),
	_version(ws_v),
	_hostPort(-1),
	closingHandshakeSent(false),
	closingHandshakeReceived(false),
	_secured(false)
{
	initTcpSocket();
}

QWsSocket::~QWsSocket()
{
	delete _currentFrame;

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

void QWsSocket::initTcpSocket()
{
	if (tcpSocket == NULL)
	{
		return;
	}

	tcpSocket->setParent(this);

	QAbstractSocket::setSocketState(tcpSocket->state());
	QAbstractSocket::setPeerAddress(tcpSocket->peerAddress());
	QAbstractSocket::setPeerPort(tcpSocket->peerPort());

	if (_version == WS_V0)
	{
		QObject::connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV0()), Qt::UniqueConnection);
	}
	else
	{
		QObject::connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV4()), Qt::UniqueConnection);
	}
	QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processTcpError(QAbstractSocket::SocketError)), Qt::UniqueConnection);
	QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)), Qt::UniqueConnection);
	QObject::connect(tcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), Qt::UniqueConnection);
	QObject::connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processTcpStateChanged(QAbstractSocket::SocketState)), Qt::UniqueConnection);
	QObject::connect(tcpSocket, SIGNAL(readChannelFinished()), this, SIGNAL(readChannelFinished()), Qt::UniqueConnection);
	QObject::connect(tcpSocket, SIGNAL(hostFound()), this, SIGNAL(hostFound()), Qt::UniqueConnection);
}

void QWsSocket::connectToHost(const QString& hostName, quint16 port, OpenMode mode)
{
	// abort connection if a the socket is not in the Unconnected state
	if (QAbstractSocket::SocketState() != QAbstractSocket::UnconnectedState)
	{
		QAbstractSocket::abort();
	}

	// trim the hostname
	_hostName = hostName.trimmed();

	// check the validity of the Websocket URI scheme
	if (!_hostName.contains(regExpUri))
	{
		return;
	}
	if (_hostName.startsWith("wss://", Qt::CaseInsensitive))
	{
		_secured = true;
	}

	// remove the websocket URI scheme for TCP connection
	_host = QString(_hostName).remove(regExpUriStart);
	_hostPort = port;
	
	// check localhost uri
	if (_host.contains(regExpLocalhostUri))
	{
		_hostAddress = QHostAddress::LocalHost;
	}
	// check IPv4 URI
	// TODO IPv6
	else if (_host.contains(QRegExp(QWsSocket::regExpIPv4)))
	{
		_hostAddress = QHostAddress(_host);
	}
	// from hostName
	else
	{
		QHostInfo info = QHostInfo::fromName(_host);
		QList<QHostAddress> hostAddresses = info.addresses();
		if (hostAddresses.size())
		{
			_hostAddress = hostAddresses.first();
		}
		else
		{
			return;
		}
	}

	if (_secured)
	{
		// replace tcpSocket by sslSocket
		tcpSocket->deleteLater();
		QSslSocket* sslSocket = new QSslSocket;
		tcpSocket = sslSocket;
		initTcpSocket();

		QObject::connect(sslSocket, SIGNAL(sslErrors(const QList<QSslError>&)), this, SIGNAL(sslErrors(const QList<QSslError>&)), Qt::UniqueConnection);

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
		sslSocket->connectToHostEncrypted(_host, port);
		sslSocket->startClientEncryption();
	}
	else
	{
		// redirected to the other function
		QWsSocket::connectToHost(_hostAddress, _hostPort, mode);
	}
}

void QWsSocket::connectToHost(const QHostAddress& address, quint16 port, OpenMode mode)
{
	if (!_secured)
	{
		handshakeResponse.clear();
		setPeerAddress(address);
		setPeerPort(port);
		setOpenMode(mode);
		tcpSocket->connectToHost(address, port, mode);
	}
	else
	{
		// redirected to the other function
		QWsSocket::connectToHost(address.toString(), port, mode);
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

void QWsSocket::close(CloseStatusCode closeStatusCode, QString reason)
{
	if (QAbstractSocket::state() == QAbstractSocket::UnconnectedState)
	{
		return;
	}

	if (! closingHandshakeSent)
	{
		switch (_version)
		{
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
					if (_wsMode == WsClientMode)
					{
						maskingKey = QWsSocket::generateMaskingKey();
					}
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
						QByteArray reason_ba = reason.toUtf8();
						if (_wsMode == WsClientMode)
						{
							reason_ba = QWsSocket::mask(reason_ba, maskingKey);
						}
						body.append(reason_ba);
					}

					BA.append(body);
				}

				// Send closing handshake
				tcpSocket->write(BA);
				tcpSocket->flush();
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
		tcpSocket->flush();
		tcpSocket->disconnectFromHost();
	}
}

qint64 QWsSocket::write(const QString& string)
{
	if (_version == WS_V0)
	{
		return QWsSocket::internalWrite(string.toUtf8(), false);
	}
	else
	{
		return QWsSocket::internalWrite(string.toUtf8(), false);
	}
}

qint64 QWsSocket::write(const QByteArray& byteArray)
{
	return QWsSocket::internalWrite(byteArray, true);
}

qint64 QWsSocket::internalWrite(const QByteArray& byteArray, bool asBinary)
{
	if (_version == WS_V0)
	{
		QByteArray BA;
		BA.append((char)0x00);
		BA.append(byteArray);
		BA.append((char)0xFF);
		return writeFrame(BA);
	}
	
	QByteArray maskingKey;
	if (_wsMode == WsClientMode)
	{
		if (_version == WS_V4)
		{
			maskingKey = QWsSocket::generateMaskingKeyV4(key, accept);
		}
		else
		{
			maskingKey = QWsSocket::generateMaskingKey();
		}
	}
	
	Opcode opcode = (asBinary ? OpBinary : OpText);
	const QList<QByteArray>& framesList = QWsSocket::composeFrames(byteArray, opcode, maskingKey, maxBytesPerFrame);

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
	
	QWsHandshake handshake(WsServerMode);
	
	if (handshake.read(tcpSocket) == false)
	{
		emit error(QAbstractSocket::ConnectionRefusedError);
		tcpSocket->abort();
		return;
	}

	if (!handshake.readStarted || !handshake.complete)
	{
		return;
	}

	// If the mandatory params are not setted, we abord the connection to the Websocket server
	if (!handshake.isValid()
		|| (_version >= WS_V4 && (QWsSocket::computeAcceptV4(key) != handshake.accept))
		|| (_version == WS_V0 && (QWsSocket::computeAcceptV0(key1, key2, key3) != handshake.accept)))
	{
		emit error(QAbstractSocket::ConnectionRefusedError);
		return;
	}
	
	accept = handshake.accept;

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
		emit frameReceived(QString::fromUtf8(currentFrame));
		currentFrame.clear();
	}

	if (tcpSocket->bytesAvailable())
	{
		processDataV0();
	}
}

void QWsSocket::processDataV4()
{
	if (state() == ConnectingState)
	{
		processHandshake();
		return;
	}

	while (true)
	{
		switch (_currentFrame->readingState)
		{
			case HeaderPending:
			{
				if (tcpSocket->bytesAvailable() < 2)
				{
					return;
				}

				// FIN, RSV1-3, Opcode
				char header[2];
				tcpSocket->read(header, 2); // XXX: Handle return value
				_currentFrame->final = (header[0] & 0x80) != 0;
				_currentFrame->rsv = header[0] & 0x70;
				_currentFrame->opcode = static_cast<Opcode>(header[0] & 0x0F);

				// Mask, PayloadLength
				_currentFrame->hasMask = (header[1] & 0x80) != 0;
				_currentFrame->payloadLength = header[1] & 0x7F;
				switch (_currentFrame->payloadLength)
				{
					case 126:
						_currentFrame->readingState = PayloadLengthPending;
						break;
					case 127:
						_currentFrame->readingState = BigPayloadLenghPending;
						break;
					default:
						_currentFrame->readingState = MaskPending;
						break;
				}
				break;
			};
			case PayloadLengthPending:
			{
				if (tcpSocket->bytesAvailable() < 2)
				{
					return;
				}

				quint16 length;
				tcpSocket->read((char*)&length, 2); // XXX: Handle return value
				_currentFrame->payloadLength = qFromBigEndian(length);
				_currentFrame->readingState = MaskPending;
				break;
			};
			case BigPayloadLenghPending:
			{
				if (tcpSocket->bytesAvailable() < 8)
				{
					return;
				}

				quint64 length;
				tcpSocket->read((char*)&length, 8); // XXX: Handle return value
				// Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
				// XXX: Check for that?
				_currentFrame->payloadLength = qFromBigEndian(length);
				_currentFrame->readingState = MaskPending;
				break;
			};
			case MaskPending:
			{
				if (!_currentFrame->hasMask)
				{
					_currentFrame->readingState = PayloadBodyPending;
					break;
				}

				if (tcpSocket->bytesAvailable() < 4)
				{
					return;
				}

				tcpSocket->read(_currentFrame->maskingKey, 4); // XXX: Handle return value

				/*if (opcode == OpClose)
				{
					_currentFrame->readingState = CloseDataPending;
				}
				else
				{*/
					_currentFrame->readingState = PayloadBodyPending;
				//}
				//break; //// Intentional fall-through ////
			};
			case PayloadBodyPending:
			{
				// TODO: Handle large payloads
				if (tcpSocket->bytesAvailable() < _currentFrame->payloadLength)
				{
					return;
				}

				_currentFrame->payload = tcpSocket->read(_currentFrame->payloadLength);
				currentFrame.append(_currentFrame->data());

				currentOpcode = _currentFrame->opcode;
				if (!_currentFrame->valid())
				{
					_currentFrame->clear();
					if (currentOpcode == OpClose)
					{
						closingHandshakeReceived = true;
					}
					close(CloseProtocolError);
					continue;
				}

				if (_currentFrame->controlFrame())
				{
					handleControlFrame();
				}
				else
				{
					if (currentOpcode != OpContinue)
					{
						currentDataOpcode = _currentFrame->opcode;
					}

					if ((currentOpcode == OpContinue && !continuation) || (currentOpcode != OpContinue && continuation))
					{
						_currentFrame->clear();
						close(CloseProtocolError);
						continue;
					}

					continuation = !_currentFrame->final;
					currentData.append(_currentFrame->data());

					if (_currentFrame->final)
					{
						handleData();
					}
				}

				_currentFrame->clear();
				break;
			};
			case CloseDataPending:
				// TODO
				break;
			default:
				break;
		} // end switch
	} // end while(1)

	if (tcpSocket->bytesAvailable())
	{
		processDataV4();
	}
}

void QWsSocket::handleData()
{
	if (state() == ClosingState)
	{
		return;
	}
	if (currentDataOpcode == OpBinary)
	{
		emit frameReceived(currentData);
		currentData.clear();
		return;
	}
	if (currentDataOpcode == OpText)
	{
		emit frameReceived(QString::fromUtf8(currentData));
		currentData.clear();
		return;
	}
}

void QWsSocket::handleControlFrame()
{
	if (currentOpcode == OpClose)
	{
		closingHandshakeReceived = true;
		close(NoCloseStatusCode);
		return;
	}
	if (state() == ClosingState)
	{
		return;
	}
	if (currentOpcode == OpPing)
	{
		handlePing(_currentFrame->data());
		return;
	}
	if (currentOpcode == OpPong)
	{
		emit pong(pingTimer.elapsed());
		return;
	}
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
	if (_wsMode == WsClientMode)
	{
		startHandshake();
	}
}

// starting client handshake
void QWsSocket::startHandshake()
{
	if (_version == WS_V13)
	{
		key = QWsSocket::generateNonce();
		QString handshake = composeOpeningHandShakeV13("/", _host, key);
		tcpSocket->write(handshake.toUtf8());
	}
	else if (_version == WS_V0)
	{
		key1 = QWsSocket::generateKey1or2();
		key2 = QWsSocket::generateKey1or2();
		key3 = QWsSocket::generateKey3();
		QString handshake = composeOpeningHandShakeV0("/", _host, key1, key2, key3);
		tcpSocket->write(handshake.toUtf8());
	}
	else // support more version soon
	{
		QAbstractSocket::setErrorString("This protocol version in not implemented");
		emit QAbstractSocket::error();
		tcpSocket->close();
	}
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
			if (!_secured && wsSocketState == QAbstractSocket::ConnectingState)
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

void QWsSocket::processTcpError(QAbstractSocket::SocketError err)
{
	setSocketError(tcpSocket->error());
	setErrorString(tcpSocket->errorString());
	emit error(err);
}

void QWsSocket::ping()
{
	pingTimer.restart();
	QByteArray pingFrame = QWsSocket::composeHeader(true, OpPing, 0);
	writeFrame(pingFrame);
}

void QWsSocket::handlePing(QByteArray applicationData)
{
	if (applicationData.size() > 125)
	{
		return close(CloseProtocolError);
	}

	writeFrames(QWsSocket::composeFrames(applicationData, OpPong));
}

QByteArray QWsSocket::generateNonce()
{
	QByteArray nonce;

	int i = 16;
	while(i--)
	{
		nonce.append(rand8());
	}

	return nonce.toBase64();
}

QByteArray QWsSocket::generateKey1or2()
{
	QByteArray key;

	// generate spaces number
	quint32 spaces = rand8(2, 10);

	// generate a correct random number
	quint32 partMax = qFloor((double)UINT_MAX / (double)spaces);
	quint32 part = rand32(0, partMax);
	quint32 key_number = part * spaces;
	key = QByteArray::number(key_number);

	// integrate some random characters
	int i = rand8(10, 20);
	while (i--)
	{
		QChar car;
		do
		{
			car = rand8(32, 126);
		} while (car.isDigit());
		key.insert(rand8(0, key.size()), car);
	}

	// integrate spaces
	int j = spaces;
	while (j--)
	{
		key.insert(rand8(1, key.size()-1), QLatin1String(" "));
	}

	return key;
}

QByteArray QWsSocket::generateKey3()
{
	QByteArray key;

	int i = 8;
	while(i--)
	{
		key.append(rand8(32, 126));
	}

	return key;
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

QByteArray QWsSocket::generateMaskingKeyV4(QByteArray key, QByteArray nonce)
{
	QByteArray concat;
	concat += key;
	concat += nonce;
	concat += QByteArray("61AC5F19-FBBA-4540-B96F-6561F1AB40A8");
	QByteArray hash = QCryptographicHash::hash (concat, QCryptographicHash::Sha1);
	return hash;
}

QByteArray QWsSocket::computeAcceptV0(QByteArray key1, QByteArray key2, QByteArray key3)
{
	quint32 key_number_1 = QString::fromUtf8(key1).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();
	quint32 key_number_2 = QString::fromUtf8(key2).remove(QRegExp(QLatin1String("[^\\d]"))).toUInt();

	quint32 spaces_1 = key1.count(' ');
	quint32 spaces_2 = key2.count(' ');

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

QList<QByteArray> QWsSocket::composeFrames(QByteArray data, Opcode opcode, QByteArray maskingKey, int maxFrameBytes)
{
	if (maxFrameBytes == 0)
	{
		maxFrameBytes = maxBytesPerFrame;
	}

	QList<QByteArray> frames;
	int nbFrames = (data.size() / maxFrameBytes) + 1;

	for (int i=0; i<nbFrames; i++)
	{
		QByteArray frame;

		// opCode
		Opcode frameOpcode = OpContinue;
		if (i == 0)
		{
			frameOpcode = opcode;
		}

		// final frame & frame size
		bool final = false;
		quint64 frameSize = maxFrameBytes;
		if (i == nbFrames-1) // for multi-frames
		{
			final = true;
			frameSize = data.size();
		}

		// Compose and append the header to the frame
		frame.append(QWsSocket::composeHeader(final, frameOpcode, frameSize, maskingKey));
		
		// Application Data
		QByteArray frameData = data.left(frameSize);
		data.remove(0, frameSize);
		
		// mask frame data if necessary
		if (maskingKey.size())
		{
			frameData = QWsSocket::mask(frameData, maskingKey);
		}

		// append payload to the frame
		frame.append(frameData);

		// append frame to framesList
		frames << frame;
	}
	return frames;
}

QByteArray QWsSocket::composeHeader(bool end, Opcode opcode, quint64 payloadLength, QByteArray maskingKey)
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

QString QWsSocket::composeOpeningHandShakeV13(QString resourceName, QString host, QByteArray key, QString origin, QString protocol, QString extensions)
{
	QString hs;
	hs += QString("GET %1%2 HTTP/1.1\r\n").arg(resourceName).arg(resourceName.endsWith('/') ? "" : "/");
	hs += QString("Host: %1\r\n").arg(host);
	hs += QLatin1String("Upgrade: websocket\r\n");
	hs += QLatin1String("Connection: Upgrade\r\n");
	hs += QString("Sec-WebSocket-Key: %1\r\n").arg(QLatin1String(key));
	hs += QLatin1String("Sec-WebSocket-Version: 13\r\n");
	if (!origin.isEmpty())
	{
		hs += QString("Origin: %1\r\n").arg(origin);
	}
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

QString QWsSocket::composeOpeningHandShakeV0(QString resourceName, QString host, QByteArray key1, QByteArray key2, QByteArray key3, QString origin, QString protocol, QString extensions)
{
	QString hs;
	hs += QString("GET %1 HTTP/1.1\r\n").arg(resourceName);
	hs += QString("Host: %1\r\n").arg(host);
	hs += QLatin1String("Upgrade: websocket\r\n");
	hs += QLatin1String("Connection: Upgrade\r\n");
	hs += QString("Sec-WebSocket-Key1: %1\r\n").arg(QLatin1String(key1));
	hs += QString("Sec-WebSocket-Key2: %1\r\n").arg(QLatin1String(key2));
	if (!origin.isEmpty())
	{
		hs += QString("Origin: %1\r\n").arg(origin);
	}
	if (!protocol.isEmpty())
	{
		hs += QString("Sec-WebSocket-Protocol: %1\r\n").arg(protocol);
	}
	if (!extensions.isEmpty())
	{
		hs += QString("Sec-WebSocket-Extensions: %1\r\n").arg(extensions);
	}
	hs += QLatin1String("\r\n");
	hs += QLatin1String(key3);
	return hs;
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

} // namespace QtWebsocket
