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
	virtual void close( QString reason = QString() );
	qint64	write ( const QByteArray & byteArray );
	
	// Static functions
	static QString decodeFrame( QTcpSocket * socket );
	static QByteArray composeFrame( QByteArray byteArray, int maxFrameBytes = 125 );
};

#endif // QWSSOCKET_H
