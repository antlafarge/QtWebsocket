#include "SocketThread.h"

#include "Log.h"

SocketThread::SocketThread( QWsSocket * wsSocket ) :
    QThread()
{
    worker = new SocketWorker(wsSocket);

	// Move this thread object in the thread himsleft
	// Thats necessary to exec the event loop in this thread
    worker->moveToThread( this );

    // We need processEvent for finishing moveToThread procedure
    //QCoreApplication::sendPostedEvents ( 0, QEvent::ThreadChange );
    // OR
    //qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

SocketThread::~SocketThread()
{
}

void SocketThread::run()
{
    connect(worker, SIGNAL(messageReceived(QString)), this, SIGNAL(messageReceived(QString)), Qt::QueuedConnection);
    connect(worker, SIGNAL(disconnected()), this, SLOT(quit()), Qt::QueuedConnection);
    connect(this, SIGNAL(broadcastMessage(QString)), worker, SLOT(broadcastMessage(QString)), Qt::QueuedConnection);

	// Launch the event loop to exec the slots
	exec();

    delete worker;
}

SocketWorker::SocketWorker(QWsSocket *wsSocket)
    : socket(wsSocket)
{
    socket->setParent(this);

    // Connecting the socket signals here to exec the slots in the new thread
    connect( socket, SIGNAL(readyRead()), this, SLOT(processMessage()) );
    connect( socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
}

void SocketWorker::processMessage()
{
    // ANY PROCESS OF THE FRAME IS DONE IN THE SOCKET THREAD !
    QWsSocket * socket = qobject_cast<QWsSocket*>(sender());

    if ( socket == 0 )
        return;

    QWsSocketFrame frame = socket->readFrame();
    if (frame.binary || frame.data.isEmpty())
        return;

    QString message = QString::fromUtf8(frame.data);
    broadcastMessage(message);

    emit messageReceived(message);
}

void SocketWorker::broadcastMessage(QString message)
{
    socket->write(message);
}

void SocketWorker::socketDisconnected()
{
	// Prepare the socket to be deleted after last events processed
    socket->deleteLater();

    emit disconnected();
}
