# QtWebsocket

A Qt Websocket server and client implementation.
The client has been recently added in the project and is still an alpha implementation.

# Installation

**Static lib**
The QtWebsocket project should compile as a static lib.
You can easily compile it by opening the sln with Visual Studio or the pro with QtCreator.
Finaly you just have to link the static lib in your project (like the examples).

# Informations

**Supported clients**
- Google Chrome 16+
- Mozilla Firefox 9+
- Safari 5.1+
- Opera 12.50+

#Development

**Implemented functionnalities**
- Opening handshake
- Closing handshake
- Frames (send and receive)
- Mask sent frames, receive masked frames
- Control frames (ping, pong)
- Differents payload Lengths
- Multi-frames (send and receive)
- Multi-thread support
- Client implementation _(beta)_

**Todo**
- Websocket extensions
- WSS protocol


_Thanks for your interest_
