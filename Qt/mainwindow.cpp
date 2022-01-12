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


void MainWindow::on_pushButton_clicked()
{
    std::string name = ui->inputName->text().toStdString();

    if(name.length()<2){
        NameError* err = new NameError();
        err->show();
        return;
    }

    QStringList list = ui->inputAddress->text().split(':');
    std::string address = list[0].toStdString();
    int port = list[1].toInt();
    Room* room = new Room();
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

