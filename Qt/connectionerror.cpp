#include "connectionerror.h"
#include "ui_connectionerror.h"

ConnectionError::ConnectionError(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionError)
{
    ui->setupUi(this);
}

ConnectionError::~ConnectionError()
{
    delete ui;
}

void ConnectionError::on_pushButton_clicked()
{
    this->close();
}

