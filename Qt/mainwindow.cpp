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

