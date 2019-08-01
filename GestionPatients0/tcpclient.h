#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class MyTCPClient : public QObject
{
    Q_OBJECT

public:
    explicit MyTCPClient(QObject *parent = 0);

signals:

public slots:
    void readTCPData();

private:
    QTcpSocket *psocket;

};

#endif // TCPCLIENT_H
