QT       += core

QT       += gui
QT       += network

TARGET = Server
CONFIG   -= console
CONFIG   -= app_bundle


TEMPLATE = app


SOURCES += Log.cpp \
    ServerExample.cpp \
    main.cpp



HEADERS += Log.h \
    QWsServer.h \
    QWsSocket.h \
    ServerExample.h


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../QtWebSocket/release/ -lQtWebSocket
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../QtWebSocket/debug/ -lQtWebSocket
else:unix:!symbian: LIBS += -L$$OUT_PWD/../../QtWebSocket/ -lQtWebSocket

INCLUDEPATH += $$PWD/../../QtWebSocket
DEPENDPATH += $$PWD/../../QtWebSocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../QtWebSocket/release/QtWebSocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../QtWebSocket/debug/QtWebSocket.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../../QtWebSocket/libQtWebSocket.a
