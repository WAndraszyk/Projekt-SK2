#include "room.h"
#include "ui_room.h"

#define LENGTH 2048
#define NAME_LEN 32

Room::Room(QWidget *parent)
    : QWidget{parent}
{
    this->flag = 0;
    this->sockfd = 0;

    ui = new Ui::Room;
    ui->setupUi(this);
}

int Room::connectToServer(std::string ip, int port, std::string name){

    struct sockets::sockaddr_in server_addr;
    this->name = name;

    /* Ustawianie Socketa */
    sockfd = socket(AF_INET, sockets::SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = sockets::inet_addr(ip.c_str());
    server_addr.sin_port = sockets::htons(port);

    int check = sockets::connect(sockfd, (struct sockets::sockaddr *)&server_addr, sizeof(server_addr));
    if (check == -1) {
        return -1;
    }

    // Przesylamy nazwe uzytkownika serwerowi
    sockets::send(sockfd, name.c_str(), NAME_LEN, 0);

    return 0;
}

void* Room::recv_msg_handler(void){
    char message[LENGTH+NAME_LEN] = {};
    int receive = 0;

    while (1){
        receive = sockets::recv(this->sockfd, message, LENGTH, 0);

        if (receive > 0){
            printf("%s", message);
        }
        else if (receive == 0){
            break;
        }
        memset(message, 0, sizeof(message));
    }
}

void* Room::helper(void *context){

    return ((Room *)context)->recv_msg_handler();
}


void Room::listen(){

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, &helper, NULL) != 0){
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

