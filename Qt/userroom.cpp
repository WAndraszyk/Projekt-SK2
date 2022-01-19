#include "userroom.h"
#include "ui_userroom.h"

#define LENGTH 2048
#define NAME_LEN 32
#define MAX_CLIENTS 100

UserRoom::UserRoom()
{
    ui = new Ui::UserRoom;
    ui->setupUi(this);

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(on_messageReceived(QString)));
}


void UserRoom::on_sendButton_clicked()
{
    std::string message = ui->plainTextEdit->toPlainText().toStdString();
    ui->plainTextEdit->clear();

    char buffer[LENGTH + NAME_LEN] = {};

    sprintf(buffer, "%s: %s\n", this->name.c_str(), message.c_str());
    sockets::send(sockfd, buffer, strlen(buffer), 0);

    memset(buffer, 0, LENGTH + NAME_LEN);
}

void UserRoom::on_messageReceived(const QString message){
    int check = message.split('\n').length();
    if(check>1){
        QStringList list = message.split('\n');
        for(int i = 0; i<check; i++){
            if(list[i].length()>0)
            on_messageReceived(list[i]);
        }
        return;
    }
    if(message.simplified().startsWith("/USERS")){
        QStringList command = QString(message).simplified().split(' ');
        if(command[1].compare("add") == 0){
            addUser(command[2]);
        }
        else if(command[1].compare("remove") == 0){
            removeUser(command[2]);
        }
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

void UserRoom::listUsers(){
    this->ui->listWidget->clear();
    this->ui->listWidget->addItems(this->usernames);
}

void UserRoom::addUser(QString user){
    this->usernames.append(user);
    this->ui->listWidget->addItem(user);
}

void UserRoom::removeUser(QString user){
    this->usernames.removeAll(user);
    listUsers();
}
