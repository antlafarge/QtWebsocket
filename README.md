# QtWebsocket

A Qt Websocket server and client implementation.  

## Licence

Copyright 2013 Antoine Lafarge qtwebsocket@gmail.com

This file is part of QtWebsocket.

QtWebsocket is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

QtWebsocket is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QtWebsocket.  If not, see <http://www.gnu.org/licenses/>.

## Compatibility

- Google Chrome 16+  
- Mozilla Firefox 9+  
- Safari 5.1+
- Opera 12.5+

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

**Implemented functionalities**  
- Text and binary data  
- Multi-thread support  
- Server and client implementation  
- WSS protocol (HTTPS/SSL/TLS)  

**Todo**
- Websocket extensions  

_Thank you for your interest._
