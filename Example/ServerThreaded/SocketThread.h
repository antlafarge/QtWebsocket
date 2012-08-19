#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>

#include "QWsSocket.h"

class SocketThread : public QThread
{
	Q_OBJECT

public:
	SocketThread();
	~SocketThread();

	QWsSocket * socket;
	void run();

private slots:
	void onDataReceived(QString data);
	void onPong(quint64 elapsedTime);
	void onClientDisconnection();

private:
	
};

#endif // SOCKETTHREAD_H
