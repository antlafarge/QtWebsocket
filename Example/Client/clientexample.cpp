#include "clientexample.h"
#include "ui_clientexample.h"

ClientExample::ClientExample(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientExample)
{
    ui->setupUi(this);

    wsSocket = new QWsSocket( this );

    QObject::connect( ui->sendButton, SIGNAL(pressed()), this, SLOT(sendMessage()) );
    QObject::connect( ui->textLineEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()) );
    QObject::connect( ui->connectButton, SIGNAL(pressed()), this, SLOT(connectSocket()) );
    QObject::connect( ui->disconnectButton, SIGNAL(pressed()), this, SLOT(disconnectSocket()) );
    QObject::connect( wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)) );
    QObject::connect( wsSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)) );
}

ClientExample::~ClientExample()
{
    delete ui;
}

void ClientExample::sendMessage()
{
    QString message = ui->pseudoLineEdit->text() + ": " + ui->textLineEdit->text();
    ui->textLineEdit->clear();
    wsSocket->write( message );
}

void ClientExample::displayMessage( QString message )
{
    ui->chatTextEdit->append( message );
}

void ClientExample::connectSocket()
{
    wsSocket->connectToHost( QHostAddress("127.0.0.1"), 1337 );
}

void ClientExample::disconnectSocket()
{
    wsSocket->disconnect();
}

void ClientExample::socketStateChanged(QAbstractSocket::SocketState socketState)
{

    switch ( wsSocket->state() )
    {
    case QAbstractSocket::UnconnectedState:
        ui->socketStateLabel->setText("Unconnected");
        displayMessage( "DISCONNECTED" );
        break;
    case QAbstractSocket::HostLookupState:
        ui->socketStateLabel->setText("HostLookup");
        break;
    case QAbstractSocket::ConnectingState:
        ui->socketStateLabel->setText("Connecting");
        break;
    case QAbstractSocket::ConnectedState:
        ui->socketStateLabel->setText("Connected");
        displayMessage( "CONNECTED" );
        break;
    case QAbstractSocket::BoundState:
        ui->socketStateLabel->setText("Bound");
        break;
    case QAbstractSocket::ClosingState:
        ui->socketStateLabel->setText("Closing");
        break;
    case QAbstractSocket::ListeningState:
        ui->socketStateLabel->setText("Listening");
        break;
    default:
        ui->socketStateLabel->setText("Unknow");
        break;
    }
}
