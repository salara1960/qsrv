#-------------------------------------------------
#
# Project created by QtCreator 2018-11-14T10:04:46
#
#-------------------------------------------------

QT += core gui widgets network sql
# webview
QT += location positioning qml quick quickwidgets

CONFIG += gnu++14
DEFINES -= QT_DEPRECATED_WARNINGS

TARGET = gps
TEMPLATE = app

SOURCES += main.cpp\
        srv.cpp

HEADERS  += srv.h

FORMS    += srv.ui

RESOURCES += rc.qrc
