#include "room.h"
#include <QDebug>

Room::Room(QWidget *parent)
    : QWidget{parent}
{
    this->flag = 0;
    this->sockfd = 0;
}

Room::~Room(){
}

void Room::closeEvent(QCloseEvent *event){
    if(!(this->kicked)){
        sockets::send(this->sockfd, "/leaving\n", LENGTH, sockets::MSG_NOSIGNAL);
    }
    QWidget::closeEvent(event);
    this->~Room();
}

int Room::connectToServer(std::string ip, int port, std::string name){

    struct sockets::sockaddr_in server_addr;
    this->name = name;
    this->Qname = QString(name.c_str());

    /* Ustawianie Socketa */
    sockfd = sockets::socket(AF_INET, sockets::SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = sockets::inet_addr(ip.c_str());
    server_addr.sin_port = sockets::htons(port);

    int check = sockets::connect(sockfd, (struct sockets::sockaddr *)&server_addr, sizeof(server_addr));
    if (check == -1) {
        return -1;
    }

    // Przesylamy nazwe uzytkownika serwerowi
    sockets::send(sockfd, name.c_str(), NAME_LEN, sockets::MSG_NOSIGNAL);

    char message[LENGTH] = {};
    if(sockets::recv(sockfd, message, LENGTH, 0)>0){
        QString Qmessage = message;
        if(Qmessage.startsWith("/nametaken")) return -2;
    }
    else return -1;

    return 0;
}

void* Room::recv_msg_handler(void *arguments){
    char message[LENGTH] = {};
    int receive = 0;
    Room *args = (Room *)arguments;
    int fd = args->sockfd;

//    qInfo() << fd;
//    qInfo() << &fd;
//    qInfo() << *fd;

    while (1){
        receive = sockets::recv(fd, message, sizeof(message), 0);

        if (receive > 0){
//            qInfo() << message;
            emit args->messageReceived(message);
        }
        else if (receive == 0){
            break;
        }
        memset(message, 0, sizeof(message));
    }
}

void Room::listen(){

//    char message[LENGTH+NAME_LEN] = {};
//    int res = sockets::recv(sockfd, message, LENGTH+NAME_LEN, 0);
//    qInfo() << res;
//    ui->textBrowser->setPlainText(message);
//    qInfo() << sockfd;
//    qInfo() << &sockfd;

    pthread_t recv_msg_thread;

    if(pthread_create(&recv_msg_thread, NULL, (THREADFUNCPTR) Room::recv_msg_handler, this) != 0){
        printf("ERROR: pthread\n");
        return;
    }
}

void Room::on_Room_destroyed()
{
}


void Room::on_Room_customContextMenuRequested(const QPoint &pos)
{

}
