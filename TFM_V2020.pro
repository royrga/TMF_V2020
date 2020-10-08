#-------------------------------------------------
#
# Project created by QtCreator 2020-02-23T19:09:17
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TFM_V2020
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    SPIconfig.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/local/qwt-6.1.3/include

LIBS += -lpigpio -lrt
LIBS += -L/usr/local/qwt-6.1.3/lib/
