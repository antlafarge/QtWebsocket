#ifndef HEADER_WebSocketServer
#define HEADER_WebSocketServer

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"
#include "SocketThread.h"

class ServerExampleThreaded : public QObject
{
	Q_OBJECT

public:
	ServerExampleThreaded();
	~ServerExampleThreaded();

public slots:
	void processNewConnection();
	void displayMessage( QString message );

signals:
	void broadcastMessage( QString message );

private:
	QWsServer * server;
};

#endif
