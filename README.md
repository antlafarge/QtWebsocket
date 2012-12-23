# QtWebsocket

A Qt Websocket server and client implementation.  
The client has been recently added in the project and is still a beta implementation.

## Informations

**Supported clients**  
- Google Chrome 16+  
- Mozilla Firefox 9+  
- Safari 5.1+  
- Opera 12.50+

## Installation

You have two possibilities described below.  
Don't hesitate to look at the examples.

**Modular and clean method - Static lib**  
- Open the main project with QtCreator _(QtWebsocket.pro)_ or Visual Studio _(QtWebsocket.sln)_.  
- Compile the project, this will result in a static lib _(*.lib or *.a file)_.  
- Add the _header_ files _(.h)_ from the _QtWebsocket_ directory in your project.  
- Link the generated static lib to your project.  
- Compile your project !

**Easy and hard method - Add the sources to your project**  
- Copy the _QtWebsocket_ directory in your project.  
- Add the _header_ and _cpp_ files in your project.  
- Compile your project !

## Documentation

No doc for the moment, the best way is to look at the examples.  
I created it to show how you can use properly the lib.

## Development

**Implemented functionnalities**  
- Opening handshake  
- Frames (send and receive)  
- Mask sent frames, receive masked frames  
- Control frames (close, ping, pong)  
- Multi-frames (send and receive)  
- Client implementation _(beta)_

**In progress**
- WSS protocol (SSL) - experementall

**Todo**
- Websocket extensions
- Real multi-thread support (?)

_Thanks for your interest._
