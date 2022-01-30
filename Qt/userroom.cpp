#include "userroom.h"
#include "ui_userroom.h"

UserRoom::UserRoom()
{
    ui = new Ui::UserRoom;
    ui->setupUi(this);
    this->kicked = false;

    connect(this, SIGNAL(messageReceived(QString)), this, SLOT(on_messageReceived(QString)));
}

UserRoom::~UserRoom(){
    delete ui;
}

void UserRoom::on_sendButton_clicked()
{
    std::string message = ui->plainTextEdit->toPlainText().toStdString();
    ui->plainTextEdit->clear();

    char buffer[LENGTH + NAME_LEN] = {};

    sprintf(buffer, "%s: %s\n", this->name.c_str(), message.c_str());
    sockets::send(sockfd, buffer, strlen(buffer), sockets::MSG_NOSIGNAL);

    memset(buffer, 0, LENGTH + NAME_LEN);
}

void UserRoom::on_messageReceived(const QString message){
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
        if(command[1] == this->Qname){
            this->ui->textBrowser->append("Zostales wyrzucony(a) z serwera!");
            this->ui->listWidget->clear();
            this->ui->plainTextEdit->setReadOnly(true);
            this->ui->sendButton->setEnabled(false);
            sockets::close(this->sockfd);
            this->kicked = true;
        }
        else
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
        if(command[1] == owner){
            this->ui->textBrowser->setTextColor(Qt::red);
            this->ui->textBrowser->append("Wlasciciel opuscil serwer!");
            this->ui->textBrowser->append(" ");
            this->ui->textBrowser->append("Pokoj nieczynny.");
            this->ui->listWidget->clear();
            this->ui->plainTextEdit->setReadOnly(true);
            this->ui->sendButton->setEnabled(false);
            sockets::close(this->sockfd);
            this->kicked = true;
        }
        return;
    }
    if(message.simplified().startsWith("/owner")){
        QStringList command = QString(message).simplified().split(' ');
        this->owner = command[1];
        userInList();
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

void UserRoom::listUsers(){
    this->ui->listWidget->clear();
    this->ui->listWidget->addItems(this->usernames);
    userInList();
}

void UserRoom::addUser(QString user){
    this->usernames.append(user);
    this->ui->listWidget->addItem(user);
    userInList();
}

void UserRoom::removeUser(QString user){
    this->usernames.removeAll(user);
    listUsers();
}

void UserRoom::userInList(){
    for(int i = 0; i < this->ui->listWidget->count(); i++){
        if(this->ui->listWidget->item(i)->text() == this->Qname){
            this->ui->listWidget->item(i)->setForeground(Qt::blue);
        }
        else if(this->ui->listWidget->item(i)->text() == this->owner){
            this->ui->listWidget->item(i)->setForeground(Qt::red);
        }
        else{
            this->ui->listWidget->item(i)->setForeground(Qt::black);
        }
    }
}
