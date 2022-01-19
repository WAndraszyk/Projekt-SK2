#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "userroom.h"
#include "ownerroom.h"
#include "connectionerror.h"
#include "nameerror.h"
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_MainWindow_destroyed();

    void on_hostButton_clicked();

    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;
    Room room;
};
#endif // MAINWINDOW_H
