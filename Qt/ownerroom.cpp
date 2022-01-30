#include "ownerroom.h"
#include "ui_ownerroom.h"

OwnerRoom::OwnerRoom()
{
    ui = new Ui::OwnerRoom;
    ui->setupUi(this);

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(on_messageReceived(QString)));
}

OwnerRoom::~OwnerRoom(){
    delete ui;
}

void OwnerRoom::on_sendButton_clicked()
{
    std::string message = ui->plainTextEdit->toPlainText().toStdString();
    ui->plainTextEdit->clear();

    char buffer[LENGTH + NAME_LEN] = {};

    sprintf(buffer, "%s: %s\n", this->name.c_str(), message.c_str());
    sockets::send(sockfd, buffer, strlen(buffer), 0);

    memset(buffer, 0, LENGTH + NAME_LEN);
}

void OwnerRoom::on_messageReceived(const QString message){
    if(message.simplified() == ""){
        return;
    }
    if(message.contains('\n')){
        QStringList list = message.split('\n');
        for(int i = 0; i<list.length(); i++){
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
    if(message.simplified().startsWith("/kick")){
        QStringList command = QString(message).simplified().split(' ');
        this->ui->textBrowser->setTextColor(Qt::red);
        this->ui->textBrowser->append(command[1] + " zostal(a) wyrzucony(a) z serwera!");
        return;
    }
    if(message.simplified().startsWith("/join")){
        QStringList command = QString(message).simplified().split(' ');
        this->ui->textBrowser->setTextColor(Qt::gray);
        this->ui->textBrowser->append(command[1] + " dolaczyl do serwera!");
        return;
    }
    if(message.simplified().startsWith("/left")){
        QStringList command = QString(message).simplified().split(' ');
        this->ui->textBrowser->setTextColor(Qt::gray);
        this->ui->textBrowser->append(command[1] + " opuscil serwer!");
        return;
    }
    if(message.simplified().startsWith("/owner")){
        return;
    }
    if(message.simplified().startsWith(this->Qname + ": ")){
        this->ui->textBrowser->setTextColor(Qt::blue);
    }
    else{
        this->ui->textBrowser->setTextColor(Qt::black);
    }
    this->ui->textBrowser->append(message.simplified());
}

void OwnerRoom::listUsers(){
    this->ui->listWidget->clear();
    this->ui->listWidget->addItems(this->usernames);
    userInList();
}

void OwnerRoom::addUser(QString user){
    this->usernames.append(user);
    this->ui->listWidget->addItem(user);
    userInList();
}

void OwnerRoom::removeUser(QString user){
    this->usernames.removeAll(user);
    listUsers();
}

void OwnerRoom::userInList(){
    for(int i = 0; i < this->ui->listWidget->count(); i++){
        if(this->ui->listWidget->item(i)->text() == this->Qname){
            this->ui->listWidget->item(i)->setForeground(Qt::blue);
        }
        else{
            this->ui->listWidget->item(i)->setForeground(Qt::black);
        }
    }
}

void OwnerRoom::on_kickButton_clicked()
{
    QList kicknames = this->ui->listWidget->selectedItems();
    for(int i = 0; i<kicknames.length(); i++){
        char buffer[7 + NAME_LEN] = {};

        sprintf(buffer, "/kick %s\n", kicknames[i]->text().toStdString().c_str());
        sockets::send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, 7 + NAME_LEN);
    }
}

