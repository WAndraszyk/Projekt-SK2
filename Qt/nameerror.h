#ifndef NAMEERROR_H
#define NAMEERROR_H

#include <QWidget>

namespace Ui {
class NameError;
}

class NameError : public QWidget
{
    Q_OBJECT

public:
    explicit NameError(QWidget *parent = nullptr);
    ~NameError();

private slots:
    void on_pushButton_clicked();

private:
    Ui::NameError *ui;
};

#endif // NAMEERROR_H
