#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    QString Qname = ui->inputName->text();

    if(Qname.length()<2 || Qname.startsWith("/")){
        NameError* err = new NameError();
        err->show();
        return;
    }

    std::string name = Qname.toStdString();
    QStringList list = ui->inputAddress->text().split(':');
    std::string address = list[0].toStdString();
    int port = list[1].toInt();
    UserRoom* room = new UserRoom();
    int check = room->connectToServer(address, port, name);
    if(check == 0){
        room->show();
        room->listen();
        return;
    }
    else if(check == -2){
        NameError* err = new NameError();
        err->nameTaken();
        err->show();
        return;
    }
    else{
        ConnectionError* err = new ConnectionError();
        err->show();
        return;
    }
}


void MainWindow::on_MainWindow_destroyed()
{
//    QCoreApplicaton::quit();
}


void MainWindow::on_hostButton_clicked()
{
    QString Qname = ui->inputName->text();

    if(Qname.length()<2 || Qname.startsWith("/")){
        NameError* err = new NameError();
        err->show();
        return;
    }

    std::string name = Qname.toStdString();
    QStringList list = ui->inputAddress->text().split(':');
    std::string address = list[0].toStdString();
    int port = list[1].toInt();
    OwnerRoom* room = new OwnerRoom();
//    QString path = std::filesystem::current_path().c_str();
//    qInfo() << path;
//    QStringList Lpath = path.split('/');
//    Lpath.removeAt(0);
//    Lpath.removeAt(0);
//    Lpath.removeAt(0);
//    path = Lpath.join('/');
//    qInfo() << path;

    system("chmod +x serwer.out");

    pthread_t server_thread;

    char command[34 + NAME_LEN] = {};
    sprintf(command, "./serwer.out %s %d %s", address.c_str(), port, name.c_str());

//    qInfo() << &command;
//    qInfo() << command;

    if(pthread_create(&server_thread, NULL, (THREADFUNCPTR) MainWindow::launchServer, command) != 0){
        qInfo() << "ERROR LAUNCHING SERVER";
        return;
    }

    QTime time = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < time){}

    int check = room->connectToServer(address, port, name);
    if(check == 0){
        room->show();
        room->listen();
        return;
    }
    else{
        ConnectionError* err = new ConnectionError();
        err->show();
        return;
    }
}

void* MainWindow::launchServer(void *args){

    char *command = (char *) args;
//    qInfo() << command;
//    qInfo() << *command;
//    qInfo() << &command;
    strcpy(command, (char*) args);
    system(command);

    return 0;
}

