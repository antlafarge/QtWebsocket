#ifndef CLIENTEXAMPLE_H
#define CLIENTEXAMPLE_H

#include <QWidget>

#include "QWsSocket.h"

namespace Ui {
class ClientExample;
}

class ClientExample : public QWidget
{
    Q_OBJECT
    
public:
    explicit ClientExample(QWidget *parent = 0);
    ~ClientExample();

protected slots:
    void sendMessage();
    void connectSocket();
    void disconnectSocket();
    void displayMessage(QString message);
    void socketStateChanged(QAbstractSocket::SocketState socketState);

protected:
    QWsSocket * wsSocket;
    
private:
    Ui::ClientExample *ui;
};

#endif // CLIENTEXAMPLE_H
