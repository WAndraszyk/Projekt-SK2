QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

QMAKE_CXXFLAGS += -pthread
QMAKE_CFLAGS += -pthread
LIBS += -pthread

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client.cpp \
    connectionerror.cpp \
    main.cpp \
    mainwindow.cpp \
    nameerror.cpp \
    ownerroom.cpp \
    room.cpp \
    userroom.cpp

HEADERS += \
    connectionerror.h \
    mainwindow.h \
    nameerror.h \
    ownerroom.h \
    room.h \
    userroom.h

FORMS += \
    connectionerror.ui \
    mainwindow.ui \
    nameerror.ui \
    ownerroom.ui \
    userroom.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
