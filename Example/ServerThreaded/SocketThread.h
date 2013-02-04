#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>

#include "QWsSocket.h"

class SocketWorker : public QObject
{
    Q_OBJECT

public:
    SocketWorker( QWsSocket *wsSocket );

public slots:
    void processMessage();
    void socketDisconnected();

    void broadcastMessage( QString message );

signals:
    void messageReceived( QString message );

    void disconnected();

private:
    QWsSocket * socket;

};

class SocketThread : public QThread
{
	Q_OBJECT

public:
    SocketThread( QWsSocket * wsSocket );
    ~SocketThread();

	void run();

signals:
    void messageReceived( QString message );
    void broadcastMessage( QString message );

private:
    SocketWorker * worker;
	
};

#endif // SOCKETTHREAD_H
