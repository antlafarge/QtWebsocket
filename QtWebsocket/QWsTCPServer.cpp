#include "QWsTCPServer.h"
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslKey>

// Default ssl certificate and key
static const char * const DefaultSslCertificate = "-----BEGIN CERTIFICATE-----\n"\
"MIICATCCAWoCCQD9mS1dgx48vTANBgkqhkiG9w0BAQUFADBFMQswCQYDVQQGEwJB\n"\
"VTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"\
"cyBQdHkgTHRkMB4XDTEyMTIyMjE5NDAwMloXDTEzMTIyMjE5NDAwMlowRTELMAkG\n"\
"A1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"\
"IFdpZGdpdHMgUHR5IEx0ZDCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA014S\n"\
"ABK8zoX8RBk1WErHdtdJFL3wIk6DeZ0szgplb0gDuPq4f6hHcK2+UauEs/Mb6IFX\n"\
"9qgG48XZtReqJqePbE7S42AErKTXNcufmVZ8FURv/1eG4fWGgXQ5t2VVQRAfZT1S\n"\
"fMiztbvQAyWemjmbMjYSEoTkxB2T3TvJPETJgpUCAwEAATANBgkqhkiG9w0BAQUF\n"\
"AAOBgQAXRctYUW9Lz1LIVRTYWazPghViPu+jSn5/8qJxevP7x+jgAR4FrmyrTfD+\n"\
"UKWSsVMeLG++ob8CZF3O6O11wrxwhSEFv6dgOBoITPbhObz9btLQx4rnj0PWi8Te\n"\
"HBf94I53kDSFqDARH6S05T4RnVuhadCc/o27XhtZ4lSU7+tbLQ==\n"\
"-----END CERTIFICATE-----";

static const char * const DefaultSslPKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIICXQIBAAKBgQDTXhIAErzOhfxEGTVYSsd210kUvfAiToN5nSzOCmVvSAO4+rh/\n"\
"qEdwrb5Rq4Sz8xvogVf2qAbjxdm1F6omp49sTtLjYASspNc1y5+ZVnwVRG//V4bh\n"\
"9YaBdDm3ZVVBEB9lPVJ8yLO1u9ADJZ6aOZsyNhIShOTEHZPdO8k8RMmClQIDAQAB\n"\
"AoGBAJ3bBpR5afrPhAyTywxKpNczh4fvJpVoj7ZW1Sx4BTNr1CPlU787PUeA6r9x\n"\
"2mTOboxhdQFokeSwUZx2tQOzZl+Atk7tInhZzqlHcA9FVMsHzHR2nwnWRYTQ7Bmb\n"\
"/B9IUFJntUzMlBYoEyWTjSTjwPIBs5ENvlYy9kjI7ATsSbBhAkEA6UkbuWn7/kjm\n"\
"q9hlHA9jNym+CQpSzclfuQ4uDuC4AtJwfH8GO/V2/GEHoX3k8qhy/c+bBpn8wjE+\n"\
"mXBwDE5pSQJBAOfyoSyH4mUmcbJAq5KST3TanOfOhZOqWV06XfOQ8f3bgCRl37j2\n"\
"EfRCMlz+5DsKIL813OXfQ4TZWE4/uWUbuu0CQAhAS7i9JOqTjYUafEkHykyTL2OG\n"\
"d/NLYhVbiQmBrUB8TPo6S/Am+HRowipWF5j1mEud4i/TlnsP3tTygyQMSfECQG3l\n"\
"NF4P58E7DMWDBIeGkOTxq0PdQsarAHo+bEM5mp5HgJg+OFi/JdSQBKKxFduvOcK+\n"\
"t3Gmbawk+kTgxmtUTyUCQQCcp5uYWryG9ovR9WVBDDGc/79pcZOghmvseoAfWwWu\n"\
"paEF4rRfV+iTn6Cxnik2XbCsLMgmmaBk7mKYGXt97mRz\n"\
"-----END RSA PRIVATE KEY-----";

QWsTcpServer::QWsTcpServer(bool encryped, QObject *parent) :
    QTcpServer(parent),
    _encrypted(encryped)
{
    qDebug() << "Started WsServer, encryption=" << encryped;
    sslCertificate = QSslCertificate(DefaultSslCertificate);
    sslKey = QSslKey(DefaultSslPKey, QSsl::Rsa);
}

void QWsTcpServer::setCertificate(const QSslCertificate &certificate, const QSslKey &key)
{
    sslCertificate = certificate;
    sslKey = key;
}

QAbstractSocket *QWsTcpServer::nextPendingSocketConnection()
{
    if (pendingConnections.isEmpty())
        return 0;
    return pendingConnections.takeFirst();
}

void QWsTcpServer::incomingConnection(int handle)
{
    qDebug() << "New tcp connection";
    if (_encrypted)
    {
        QSslSocket *serverSocket = new QSslSocket(this);
        serverSocket->setLocalCertificate(sslCertificate);
        serverSocket->setPrivateKey(sslKey);
        if (serverSocket->setSocketDescriptor(handle))
        {
            qDebug() << "Trying to start encryption";
            connect(serverSocket, SIGNAL(encrypted()), this, SLOT(ready()));
            connect(serverSocket, SIGNAL(disconnected()), serverSocket, SLOT(deleteLater()));
            serverSocket->startServerEncryption();
        }
        else
        {
            delete serverSocket;
        }
    }
    else
    {
        QTcpSocket *serverSocket = new QTcpSocket(this);
        if (serverSocket->setSocketDescriptor(handle))
            ready(serverSocket);
        else
            delete serverSocket;
    }

}


void QWsTcpServer::ready()
{
    qDebug() << "Ecrypted";
    QAbstractSocket *socket = qobject_cast<QAbstractSocket*>(sender());
    ready(socket);
}

void QWsTcpServer::ready(QAbstractSocket *socket)
{
    if (socket == 0)
        return;
    pendingConnections.append(socket);
    emit newConnection();
}
