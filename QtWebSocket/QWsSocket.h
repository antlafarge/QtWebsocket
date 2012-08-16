#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QTime>

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

class QWsSocket : public QAbstractSocket
{
	Q_OBJECT

public:
	enum EOpcode
	{
		OpContinue = 0x0,
		OpText = 0x1,
		OpBinary = 0x2,
		OpReserved1 = 0x3,
		OpReserved2 = 0x4,
		OpReserved3 = 0x5,
		OpReserved4 = 0x6,
		OpReserved5 = 0x7,
		OpClose = 0x8,
		OpPing = 0x9,
		OpPong = 0xA,
		OpReserved6 = 0xB,
		OpReserved7 = 0xC,
		OpReserved8 = 0xD,
		OpReserved9 = 0xE,
		OpReserved10 = 0xF
	};

public:
	// ctor
	QWsSocket( QTcpSocket * socket = 0, QObject * parent = 0, EWebsocketVersion websocketVersion = WS_V13 );
	// dtor
	virtual ~QWsSocket();

	// Public methods
	qint64 write ( const QString & string, int maxFrameBytes = 0 ); // write data as text
	qint64 write ( const QByteArray & byteArray, int maxFrameBytes = 0 ); // write data as binary

public slots:
	virtual void close( QString reason = QString() );
	void ping();

signals:
	void frameReceived(QString frame);
	void frameReceived(QByteArray frame);
	void pong(quint64 elapsedTime);

protected:
	qint64 writeFrames ( QList<QByteArray> framesList );
	qint64 writeFrame ( const QByteArray & byteArray );
	void dataReceivedV0();

protected slots:
	void dataReceived();

private slots:
	// private func
	void tcpSocketAboutToClose();
	void tcpSocketDisconnected();
private:
	enum EState
	{
		HeaderPending,
		PayloadLengthPending,
		BigPayloadLenghPending,
		MaskPending,
		PayloadBodyPending
	};

private:
	// private vars
	QTcpSocket * tcpSocket;
	QByteArray currentFrame;
	QTime pingTimer;
	EWebsocketVersion websocketVersion;

	EState state;
	EOpcode opcode;
	bool isFinalFragment;
	bool hasMask;
	quint64 payloadLength;
	QByteArray maskingKey;
public:
	// Static functions
	static QByteArray generateMaskingKey();
	static QByteArray mask( QByteArray data, QByteArray maskingKey );
	static QList<QByteArray> composeFrames( QByteArray byteArray, bool asBinary = false, int maxFrameBytes = 0 );
	static QByteArray composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );
	static QString composeOpeningHandShake( QString ressourceName, QString host, QString origin, QString extensions, QString key );

	// static vars
	static int maxBytesPerFrame;
};

#endif // QWSSOCKET_H
