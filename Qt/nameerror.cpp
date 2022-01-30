#include "nameerror.h"
#include "ui_nameerror.h"

NameError::NameError(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameError)
{
    ui->setupUi(this);
    this->ui->label->setAlignment(Qt::AlignHCenter);
    this->ui->label_2->setAlignment(Qt::AlignHCenter);
}

NameError::~NameError()
{
    delete ui;
}

void NameError::on_pushButton_clicked()
{
    this->close();
}

void NameError::nameTaken(){
    this->ui->label->setText("Podana nazwa jest juz zajeta");
    this->ui->label_2->setText("");
}
