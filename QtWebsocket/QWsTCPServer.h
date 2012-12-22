#ifndef QWSTCPSERVER_H
#define QWSTCPSERVER_H

#include <QTcpServer>
#include <QSslCertificate>
#include <QSslKey>

class QAbstractSocket;

class QWsTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QWsTcpServer( bool encryped = false, QObject *parent = 0 );
    void setCertificate( const QSslCertificate &certificate, const QSslKey &key );
    QAbstractSocket *nextPendingSocketConnection();

private:
    void incomingConnection(int handle);
    void ready(QAbstractSocket *socket);
    
signals:
    void newConnection();
    
private slots:
    void ready();

private:
    QList<QAbstractSocket*> pendingConnections;

    bool _encrypted;

    QSslKey sslKey;
    QSslCertificate sslCertificate;
    
};

#endif // QWSTCPSERVER_H
