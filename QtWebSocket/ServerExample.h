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

    private slots:
        void onClientConnection();
        void onDataReceived(QString data);
        void onClientDisconnection();

    private:
        QWsServer * server;
        QList<QWsSocket*> clients;
};

#endif
