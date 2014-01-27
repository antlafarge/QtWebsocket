TEMPLATE = subdirs

SUBDIRS += \
    QtWebsocket \
    Example/Client \
    Example/Server \
    Example/ServerThreaded \
    AutobahnTestSuite

Client.depends = QtWebsocket
Server.depends = QtWebsocket
ServerThreaded.depends = QtWebsocket
AutobahnTestSuite.depends = QtWebsocket
