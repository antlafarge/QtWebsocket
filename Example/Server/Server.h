#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include <QtNetwork>

#include "QWsServer.h"
#include "QWsSocket.h"

class Server : public QObject
{
	Q_OBJECT

public:
	Server();
	~Server();

public slots:
	void processNewConnection();
	void processMessage(QString message);
	void processPong(quint64 elapsedTime);
	void socketDisconnected();

private:
	QWsServer* server;
	QList<QWsSocket*> clients;
};

#endif // SERVER_H
