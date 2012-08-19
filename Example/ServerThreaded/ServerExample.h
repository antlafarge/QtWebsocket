#ifndef HEADER_WebSocketServer
#define HEADER_WebSocketServer

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"
#include "SocketThread.h"

class ServerExample : public QObject
{
	Q_OBJECT

public:
	ServerExample();
	~ServerExample();

public slots:
	void onClientConnection();

private:
	QWsServer * server;
};

#endif
