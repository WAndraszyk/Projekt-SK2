#ifndef USERROOM_H
#define USERROOM_H

#include "room.h"


QT_BEGIN_NAMESPACE
namespace Ui { class UserRoom; }
QT_END_NAMESPACE

class UserRoom : public Room
{
    Q_OBJECT
public:
    UserRoom();

private slots:

    void on_sendButton_clicked();

    void on_messageReceived(const QString message);

private:

    void addUser(QString user);

    void removeUser(QString user);

    void listUsers();

    Ui::UserRoom *ui;
};

#endif // USERROOM_H
