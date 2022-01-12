#include "nameerror.h"
#include "ui_nameerror.h"

NameError::NameError(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameError)
{
    ui->setupUi(this);
}

NameError::~NameError()
{
    delete ui;
}

void NameError::on_pushButton_clicked()
{
    this->close();
}

