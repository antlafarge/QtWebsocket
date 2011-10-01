#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QTcpSocket>

class QWsSocket : public QTcpSocket
{
	Q_OBJECT

public:
	enum EOpcode
	{
		OpContinue = 0x0,
		OpText = 0x1,
		OpBinary = 0x2,
		OpClose = 0x8,
		OpPing = 0x9,
		OpPong = 0xA
	};

public:
	// ctor
	QWsSocket(QObject * parent);
	// dtor
	virtual ~QWsSocket();

	// Public methods
	QByteArray readFrame();
	qint64	write ( const QByteArray & byteArray, int maxFrameBytes = 0 );
	qint64	writeFrames ( QList<QByteArray> framesList );
	virtual void close( QString reason = QString() );

public slots:
	void dataReceived();
	void aboutToClose();

signals:
	void frameReceived();
	
public:
	// Static functions
	static QByteArray generateMaskingKey();
	static QByteArray mask( QByteArray data, QByteArray maskingKey );
	static QByteArray decodeFrame( QWsSocket * socket );
	static QList<QByteArray> composeFrames( QByteArray byteArray, int maxFrameBytes = 0 );
	static QByteArray composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );

private:
	QByteArray currentFrame;
	static int maxBytesPerFrame;
};

#endif // QWSSOCKET_H
