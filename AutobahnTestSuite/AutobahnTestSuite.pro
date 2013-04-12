QT       += core
QT       += network

TARGET   = TestServer
CONFIG   += console
#CONFIG   -= app_bundle

TEMPLATE = app

DEPENDPATH += "../QtWebsocket"
SOURCES += \
    main.cpp \
    QWsSocket.cpp \
    QWsServer.cpp \
    QWsFrame.cpp \
    TestServer.cpp

INCLUDEPATH += "../QtWebsocket"
HEADERS += \
    QWsServer.h \
    QWsSocket.h \
    QWsFrame.h \
    TestServer.h

test.commands = ./autobahntest.sh
test.target = test
test.depends = TestServer

QMAKE_EXTRA_TARGETS += test
