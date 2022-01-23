#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "userroom.h"
#include "ownerroom.h"
#include "connectionerror.h"
#include "nameerror.h"
#include <QVector>
#include <unistd.h>
#include <unistd.h>
#include <stdio.h>
#include <filesystem>
#include <QDebug>
#include <future>
#include <QTime>

#define LENGTH 2048
#define NAME_LEN 32
#define MAX_CLIENTS 100

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef void * (*THREADFUNCPTR)(void *);

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

    static void* launchServer(void* args);

    Ui::MainWindow *ui;

    Room room;
};
#endif // MAINWINDOW_H
