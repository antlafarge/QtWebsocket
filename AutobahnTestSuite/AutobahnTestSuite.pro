QT += core network

TARGET   = TestServer
CONFIG   += console
TEMPLATE = app

DEPENDPATH += "../QtWebsocket"
SOURCES += \
				main.cpp \
    TestServer.cpp

INCLUDEPATH += "../QtWebsocket"
HEADERS += \
    QWsServer.h \
				QWsSocket.h \
    TestServer.h

win32:CONFIG(release, debug|release): LIBS += -L../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../QtWebsocket/debug/ -lQtWebsocket
else:unix:!symbian: LIBS += -L../QtWebsocket/ -lQtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../QtWebsocket/release/libQtWebsocket.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../QtWebsocket/debug/libQtWebsocket.a
else:unix:!symbian: PRE_TARGETDEPS += ../QtWebsocket/libQtWebsocket.a

test.commands = ./autobahntest.sh
test.target = test
test.depends = TestServer

QMAKE_EXTRA_TARGETS += test
