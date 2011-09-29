#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QAbstractSocket>

class QWsSocket : public QAbstractSocket
{
	Q_OBJECT

public:
	// ctor
	QWsSocket(QObject * parent);
	// dtor
	virtual ~QWsSocket();
	
	// Static functions
	static QString decodeFrame( QAbstractSocket * socket );
	static QByteArray composeFrame( QString message, int maxFrameBytes = 1024 );
};

#endif // QWSSOCKET_H
