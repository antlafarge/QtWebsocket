#-------------------------------------------------
#
# Project created by QtCreator 2012-03-05T10:38:43
#
#-------------------------------------------------

QT += network

QT -= gui

TARGET = QtWebsocket
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    QWsServer.cpp \
				QWsSocket.cpp \
    QWsHandshake.cpp \
				QWsFrame.cpp \
    QTlsServer.cpp \
				functions.cpp

HEADERS += \
    QWsServer.h \
    QWsSocket.h \
				QWsHandshake.h \
				QWsFrame.h \
    QTlsServer.h \
				functions.h \
    WsEnums.h
