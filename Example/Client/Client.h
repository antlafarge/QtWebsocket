#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>

#include "QWsSocket.h"

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT
    
public:
    explicit Client(QWidget *parent = 0);
    ~Client();

protected slots:
    void sendMessage();
    void connectSocket();
    void disconnectSocket();
    void displayMessage(QString message);
    void socketStateChanged(QAbstractSocket::SocketState socketState);

protected:
    QWsSocket * wsSocket;
    
private:
    Ui::Client *ui;
};

#endif // CLIENT_H
