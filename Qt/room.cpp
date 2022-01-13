#include "room.h"
#include "ui_room.h"
#include <QDebug>
#include <errno.h>

#define LENGTH 2048
#define NAME_LEN 32
#define MAX_CLIENTS 100

struct arg_struct {
    int *fd;
    QTextBrowser **ui;
};

Room::Room(QWidget *parent)
    : QWidget{parent}
{
    this->flag = 0;
    this->sockfd = 0;

    ui = new Ui::Room;
    ui->setupUi(this);
    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(on_messageReceived(QString)));
}

int Room::connectToServer(std::string ip, int port, std::string name){

    struct sockets::sockaddr_in server_addr;
    this->name = name;

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
    sockets::send(sockfd, name.c_str(), NAME_LEN, 0);

    char users[NAME_LEN*(MAX_CLIENTS+1)+12] = {};

    check = sockets::recv(sockfd, users, NAME_LEN*(MAX_CLIENTS+1)+12, 0);
    if (check > 0){

        this->usernames = QString(users).split(',');
        this->usernames.erase(this->usernames.begin());
        listUsers();
    }


    return 0;
}

void* Room::recv_msg_handler(void *arguments){
    char message[NAME_LEN*(MAX_CLIENTS+1)+12] = {};
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
    char buffer[10 + NAME_LEN] = {};
    sprintf(buffer, "%s exits!\n", this->name.c_str());

    sockets::send(sockfd, buffer, strlen(buffer),0);
    sockets::close(sockfd);
}


void Room::on_Room_customContextMenuRequested(const QPoint &pos)
{

}


void Room::on_pushButton_clicked()
{
    std::string message = ui->plainTextEdit->toPlainText().toStdString();
    ui->plainTextEdit->clear();

    char buffer[LENGTH + NAME_LEN] = {};

    sprintf(buffer, "%s: %s\n", this->name.c_str(), message.c_str());
    sockets::send(sockfd, buffer, strlen(buffer), 0);

    memset(buffer, 0, LENGTH + NAME_LEN);
}

void Room::on_messageReceived(const QString message){
    if(message.simplified().startsWith("[USERNAMES],")){
        this->usernames = QString(message).split(',');
        this->usernames.erase(this->usernames.begin());
        listUsers();
        return;
    }
    if(message.simplified().startsWith(this->name.c_str())){
        this->ui->textBrowser->setTextColor(Qt::red);
    }
    else{
        this->ui->textBrowser->setTextColor(Qt::blue);
    }
    this->ui->textBrowser->append(message.simplified());
}

void Room::listUsers(){
    this->ui->listWidget->clear();
    this->ui->listWidget->addItems(this->usernames);
}
