#ifndef QWSSERVER_H
#define QWSSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QQueue>
#include <QNetworkProxy>

#include "QWsSocket.h"

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject * parent = 0);
	// dtor
	virtual ~QWsServer();

	// public functions
	void close();
	QString errorString();
	bool hasPendingConnections();
	bool isListening();
	bool listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
	int maxPendingConnections();
	virtual QWsSocket * nextPendingConnection();
	QNetworkProxy proxy();
	QHostAddress serverAddress();
	QAbstractSocket::SocketError serverError();
	quint16 serverPort();
	void setMaxPendingConnections( int numConnections );
	void setProxy( const QNetworkProxy & networkProxy );
	bool setSocketDescriptor( int socketDescriptor );
	int socketDescriptor();
	bool waitForNewConnection( int msec = 0, bool * timedOut = 0 );

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection( QWsSocket * socket );
	virtual void incomingConnection( int socketDescriptor );

private:
	// private methods
	//void treatSocketError();
	QString computeAcceptV8( QString key );

private slots:
	// private slots
	void newTcpConnection();
	void dataReceived();

private:
	// private attributes
	QTcpServer * tcpServer;
	//QAbstractSocket::SocketError serverSocketError;
	//QString serverSocketErrorString;
	QQueue<QWsSocket*> pendingConnections;

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
