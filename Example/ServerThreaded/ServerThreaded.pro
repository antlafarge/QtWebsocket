QT       += core
QT       += gui
QT       += network

TARGET = ServerThreaded
CONFIG   -= console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += Log.cpp \
    ServerThreaded.cpp \
    main.cpp \
    SocketThread.cpp

HEADERS += Log.h \
    QWsServer.h \
    QWsSocket.h \
    ServerThreaded.h \
    SocketThread.h

win32:CONFIG(release, debug|release): LIBS += -L../../QtWebSocket/release/ -lQtWebSocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../../QtWebSocket/debug/ -lQtWebSocket
else:unix:!symbian: LIBS += -L../../QtWebSocket/ -lQtWebSocket

INCLUDEPATH += ../../QtWebSocket
DEPENDPATH += ../../QtWebSocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../../QtWebSocket/release/QtWebSocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../../QtWebSocket/debug/QtWebSocket.lib
else:unix:!symbian: PRE_TARGETDEPS += ../../QtWebSocket/libQtWebSocket.a
