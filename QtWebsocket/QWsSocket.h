#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QTcpSocket>
#include <QSslSocket>
#include <QHostAddress>
#include <QTime>
#include <QQueue>

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

struct QWsSocketFrame
{
    bool binary;
    QByteArray data;
};

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
    QWsSocket( QObject * parent = 0,
               QAbstractSocket * socket = 0,
               bool encrypted = false,
               EWebsocketVersion ws_v = WS_V13 );
	// dtor
	virtual ~QWsSocket();

	// Public methods
    EWebsocketVersion version() const;
    QString resourceName() const;
    QString host() const;
    QString hostAddress() const;
    int hostPort() const;
    QString origin() const;
    QString protocol() const;
    QString extensions() const;

    void setResourceName( const QString & rn );
    void setHost( const QString & h );
    void setHostAddress(const QString & ha );
	void setHostPort( int hp );
    void setOrigin( const QString & o );
    void setProtocol( const QString & p );
    void setExtensions( const QString & e );

    QWsSocketFrame readFrame();

	qint64 write( const QString & string ); // write data as text
	qint64 write( const QByteArray & byteArray ); // write data as binary

    int framesAvailable() const;

public slots:
	void connectToHost( const QString & hostName, quint16 port, OpenMode mode = ReadWrite );
    void connectToHost( const QHostAddress & address, quint16 port, OpenMode mode = ReadWrite );
    void disconnectFromHost();
    void abort( QString reason = QString() );
	void ping();

signals:
    void readyRead();
	void pong(quint64 elapsedTime);

protected:
	qint64 writeFrames ( const QList<QByteArray> & framesList );
	qint64 writeFrame ( const QByteArray & byteArray );

protected slots:
	virtual void close( ECloseStatusCode closeStatusCode = NoCloseStatusCode, QString reason = QString() );
	void processDataV0();
	void processDataV4();
    void processHandshake();
	void processTcpStateChanged( QAbstractSocket::SocketState socketState );

    void onEncrypted();

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
    QAbstractSocket * tcpSocket;
    bool _encryped;
    QByteArray currentFrame;
    QQueue<QWsSocketFrame> frameBuffer;
	QTime pingTimer;

	EWebsocketVersion _version;
	QString _resourceName;
	QString _host;
	QString _hostAddress;
	int _hostPort;
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
    QString handshakeResponse;
    QString key;

public:
	// Static functions
	static QByteArray generateMaskingKey();
    static QByteArray generateMaskingKeyV4( const QString &key, const QString &nonce );
	static QByteArray mask( QByteArray & data, QByteArray & maskingKey );
	static QList<QByteArray> composeFrames( QByteArray byteArray, bool asBinary = false, int maxFrameBytes = 0 );
	static QByteArray composeHeader( bool end, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );
    static QString composeOpeningHandShake( const QString &resourceName,
                                            const QString &host,
                                            const QString &origin,
                                            const QString &extensions,
                                            const QString &key );

	// static vars
	static int maxBytesPerFrame;
};

#endif // QWSSOCKET_H
