#ifndef HEADER_WebSocketServer
#define HEADER_WebSocketServer

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"

class ServerExample : public QObject
{
	Q_OBJECT

public:
	ServerExample();
	~ServerExample();

public slots:
	void processNewConnection();
	void processMessage( QString message );
	void processPong( quint64 elapsedTime );
	void socketDisconnected();

private:
	QWsServer * server;
	QList<QWsSocket*> clients;
};

#endif
