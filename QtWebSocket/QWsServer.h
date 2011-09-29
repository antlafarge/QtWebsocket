#ifndef QWSSERVER_H
#define QWSSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QQueue>

#include "QWsSocket.h"

class QWsServer : public QTcpServer
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject * parent = 0);
	// dtor
	virtual ~QWsServer();

	// public methods
	bool listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
	void close();
	QAbstractSocket::SocketError serverError();
	QString errorString();
	bool hasPendingConnections();
	virtual QTcpSocket * nextPendingConnection();

protected:
	// Protected methods
	void addPendingConnection( QTcpSocket * socket );
	void incomingConnection( int socketDescriptor );

signals:
	// signals
	void newConnection();

private:
	// private methods
	void treatSocketError();
	QString computeAcceptV8(QString key);

private slots:
	// private slots
	void newTcpConnection();
	void dataReceived();

private:
	// private attributes
    QTcpServer * tcpServer;
	QAbstractSocket::SocketError serverSocketError;
	QString serverSocketErrorString;
	QQueue<QTcpSocket*> pendingConnections;

public:
	// public static vars
	static const QString regExpResourceNameStr;
	static const QString regExpHostStr;
	static const QString regExpKeyStr;
	static const QString regExpVersionStr;
	static const QString regExpOriginStr;
	static const QString regExpProtocolStr;
	static const QString regExpExtensionsStr;
};

#endif // QWSSERVER_H
