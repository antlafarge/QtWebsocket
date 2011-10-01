#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QTcpSocket>

class QWsServer;

class QWsSocket : public QTcpSocket
{
	Q_OBJECT

public:
	// ctor
	QWsSocket(QObject * parent);
	// dtor
	virtual ~QWsSocket();

	// Public methods
	QByteArray readFrame();
	qint64	write ( const QByteArray & byteArray );
	virtual void close( QString reason = QString() );

public slots:
	void dataReceived();
	void aboutToClose();

signals:
	void frameReceived();
	
public:
	// Static functions
	static QByteArray generateRandomMask();
	static QByteArray decodeFrame( QWsSocket * socket );
	static QByteArray composeFrame( QByteArray byteArray, int maxFrameBytes = 125 );

private:
	QByteArray currentFrame;
};

#endif // QWSSOCKET_H
