#ifndef OWNERROOM_H
#define OWNERROOM_H

#include "room.h"


QT_BEGIN_NAMESPACE
namespace Ui { class OwnerRoom; }
QT_END_NAMESPACE

class OwnerRoom : public Room
{
    Q_OBJECT
public:
    explicit OwnerRoom();

private slots:

    void on_sendButton_clicked();

    void on_messageReceived(const QString message);

    void on_kickButton_clicked();

private:

    void addUser(QString user);

    void removeUser(QString user);

    void listUsers();

    Ui::OwnerRoom *ui;
};

#endif // OWNERROOM_H
