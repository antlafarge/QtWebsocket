#-------------------------------------------------
#
# Project created by QtCreator 2012-10-11T11:39:54
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += network

TARGET = Client
CONFIG   -= console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp\
        clientexample.cpp

HEADERS  += clientexample.h \
    ../../QtWebSocket/QWsSocket.h \
    ../../QtWebSocket/QWsServer.h

FORMS    += clientexample.ui

win32:CONFIG(release, debug|release): LIBS += -L../../QtWebSocket/release/ -lQtWebSocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../../QtWebSocket/debug/ -lQtWebSocket
else:unix:!symbian: LIBS += -L../../QtWebSocket/ -lQtWebSocket

INCLUDEPATH += ../../QtWebSocket
DEPENDPATH += ../../QtWebSocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../../QtWebSocket/release/QtWebSocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../../QtWebSocket/debug/QtWebSocket.lib
else:unix:!symbian: PRE_TARGETDEPS += ../../QtWebSocket/libQtWebSocket.a
