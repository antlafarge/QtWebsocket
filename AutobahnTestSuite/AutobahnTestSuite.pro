QT       += core
QT       += network

TARGET   = TestServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    ../QtWebSocket/QWsSocket.cpp \
    ../QtWebSocket/QWsServer.cpp \
    TestServer.cpp

HEADERS += \
    QWsServer.h \
    QWsSocket.h \
    TestServer.h


INCLUDEPATH += $$PWD/../QtWebSocket
DEPENDPATH += $$PWD/../QtWebSocket

test.commands = doxygen Doxyfile; \
    test -d doxydoc/html/images || mkdir doxydoc/html/images; \
    cp documentation/images/* doxydoc/html/images
