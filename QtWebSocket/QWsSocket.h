#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QTcpSocket>
#include <QTime>

class QWsSocket : public QTcpSocket
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
	QWsSocket(QObject * parent);
	// dtor
	virtual ~QWsSocket();

	// Public methods
	QByteArray readFrame();
	qint64 write ( const QByteArray & byteArray, int maxFrameBytes = 0 );
	qint64 writeFrames ( QList<QByteArray> framesList );
	virtual void close( QString reason = QString() );
	void ping();

protected:
	qint64 writeFrame ( const QByteArray & byteArray );

public slots:
	void dataReceived();
	void aboutToClose();

signals:
	void frameReceived(QString content);
	void pong(quint64 elapsedTime);
	
public:
	// Static functions
	static QByteArray generateMaskingKey();
	static QByteArray mask( QByteArray data, QByteArray maskingKey );
	static QList<QByteArray> composeFrames( QByteArray byteArray, int maxFrameBytes = 0 );
	static QByteArray composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );

private:
	QByteArray currentFrame;
	static int maxBytesPerFrame;
	QTime pingTimer;
};

#endif // QWSSOCKET_H
