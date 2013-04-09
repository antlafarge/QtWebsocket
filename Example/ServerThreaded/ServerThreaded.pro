#-------------------------------------------------
#
# Project created by QtCreator 2013-02-23T08:36:29
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServerThreaded
TEMPLATE = app

INCLUDEPATH += ../../QtWebsocket
DEPENDPATH += ../../QtWebsocket

SOURCES += \
				main.cpp \
    Log.cpp \
    ServerThreaded.cpp \
    SocketThread.cpp

HEADERS += \
				Log.h \
    QWsServer.h \
    QWsSocket.h \
    ServerThreaded.h \
    SocketThread.h

win32:CONFIG(release, debug|release): LIBS += -L../../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../../QtWebsocket/debug/ -lQtWebsocket
else:unix:!symbian: LIBS += -L../../QtWebsocket/ -lQtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/release/libQtWebsocket.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/debug/libQtWebsocket.a
else:unix:!symbian: PRE_TARGETDEPS += ../../QtWebsocket/libQtWebsocket.a
