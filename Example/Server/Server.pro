#-------------------------------------------------
#
# Project created by QtCreator 2013-02-23T07:57:09
#
#-------------------------------------------------

QT += core network

TARGET = Server
CONFIG += console
TEMPLATE = app

DEPENDPATH += "../../QtWebsocket"
SOURCES += \
    main.cpp \
    Server.cpp

INCLUDEPATH += "../../QtWebsocket"
HEADERS += \
    QWsServer.h \
    QWsSocket.h \
    Server.h

win32:CONFIG(release, debug|release): LIBS += -L../../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../../QtWebsocket/debug/ -lQtWebsocket
else:unix:!symbian: LIBS += -L../../QtWebsocket/ -lQtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/release/libQtWebsocket.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../../QtWebsocket/debug/libQtWebsocket.a
else:unix:!symbian: PRE_TARGETDEPS += ../../QtWebsocket/libQtWebsocket.a
