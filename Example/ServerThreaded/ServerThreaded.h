#ifndef SERVERTHREADED_H
#define SERVERTHREADED_H

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"
#include "SocketThread.h"

class ServerThreaded : public QObject
{
	Q_OBJECT

public:
	ServerThreaded();
	~ServerThreaded();

public slots:
    void displayMessage( QString message );

private slots:
    void processNewConnection();
    void deleteThread();

signals:
	void broadcastMessage( QString message );

private:
	QWsServer * server;
    QList<SocketThread*> socketThreads;
};

#endif // SERVERTHREADED_H
