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


class Room : public QWidget
{
    Q_OBJECT
    std::string name;
    volatile sig_atomic_t flag;

public:
    int sockfd;
    explicit Room(QWidget *parent = nullptr);
    int connectToServer(std::string, int, std::string);
    void listen();

private slots:

    void on_Room_destroyed();

    void on_Room_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_clicked();

    void* recv_msg_handler(void);

    static void* helper(void*);

private:
    Ui::Room *ui;

};

#endif // ROOM_H
