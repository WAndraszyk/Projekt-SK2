#ifndef CONNECTIONERROR_H
#define CONNECTIONERROR_H

#include <QWidget>

namespace Ui {
class ConnectionError;
}

class ConnectionError : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionError(QWidget *parent = nullptr);
    ~ConnectionError();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ConnectionError *ui;
};

#endif // CONNECTIONERROR_H
