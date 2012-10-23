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
	void processNewConnection();
	void displayMessage( QString message );

signals:
	void broadcastMessage( QString message );

private:
	QWsServer * server;
};

#endif // SERVERTHREADED_H
