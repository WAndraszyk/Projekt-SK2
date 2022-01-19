#ifndef ROOM_H
#define ROOM_H

#include <QWidget>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>


namespace sockets{
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
}

QT_BEGIN_NAMESPACE
namespace Ui { class Room; }
QT_END_NAMESPACE

typedef void * (*THREADFUNCPTR)(void *);

class Room : public QWidget
{
    Q_OBJECT

    volatile sig_atomic_t flag;

    void listUsers();

public:
    int sockfd;
    explicit Room(QWidget *parent = nullptr);
    int connectToServer(std::string, int, std::string);
    void listen();

private slots:

    void on_Room_destroyed();

    void on_Room_customContextMenuRequested(const QPoint &pos);

signals:
    void messageReceived(const QString message);

protected:

    std::string name;

    QStringList usernames;

    static void* recv_msg_handler(void *);

};

#endif // ROOM_H
