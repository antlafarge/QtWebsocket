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
	QString readFrame();
	qint64	write ( const QByteArray & byteArray );
	virtual void close( QString reason = QString() );
	
public:
	// Static functions
	static QString decodeFrame( QTcpSocket * socket );
	static QByteArray composeFrame( QByteArray byteArray, int maxFrameBytes = 125 );
};

#endif // QWSSOCKET_H
