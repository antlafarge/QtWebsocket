QT       += core
QT       += network

TARGET   = TestServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    ../QtWebsocket/QWsSocket.cpp \
    ../QtWebsocket/QWsServer.cpp \
    ../QtWebsocket/QWsFrame.cpp \
    TestServer.cpp

HEADERS += \
    ../QtWebsocket/QWsServer.h \
    ../QtWebsocket/QWsSocket.h \
    ../QtWebsocket/QWsFrame.h \
    TestServer.h


INCLUDEPATH += $$PWD/../QtWebsocket
DEPENDPATH += $$PWD/../QtWebsocket

test.commands = doxygen Doxyfile; \
    test -d doxydoc/html/images || mkdir doxydoc/html/images; \
    cp documentation/images/* doxydoc/html/images
