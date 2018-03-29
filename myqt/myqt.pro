#-------------------------------------------------
#
# Project created by QtCreator 2018-03-19T10:22:01
#
#-------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myqt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui \
        syszuxpinyin.ui

LIBS += -lsyszuxpinyin
