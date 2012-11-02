#-------------------------------------------------
#
# Project created by QtCreator 2012-10-11T11:39:54
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += network
contains(QT_VERSION, ^5\\..*) {
QT       += widgets
}

TARGET = Client
CONFIG   -= console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp\
        Client.cpp

HEADERS  += Client.h \
    ../../QtWebsocket/QWsSocket.h \
    ../../QtWebsocket/QWsServer.h

FORMS    += Client.ui

win32:CONFIG(release, debug|release): LIBS += -L../../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../../QtWebsocket/debug/ -lQtWebsocket
else:unix:!symbian: LIBS += -L../../QtWebsocket/ -lQtWebsocket

INCLUDEPATH += ../../QtWebsocket
DEPENDPATH += ../../QtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/release/QtWebsocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/debug/QtWebsocket.lib
else:unix:!symbian: PRE_TARGETDEPS += ../../QtWebsocket/libQtWebsocket.a
