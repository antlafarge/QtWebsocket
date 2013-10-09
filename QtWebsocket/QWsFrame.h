/*
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
*/

#ifndef QWSFRAME_H
#define QWSFRAME_H

#include "WsEnums.h"

#include <QByteArray>

namespace QtWebsocket
{
	
/*!
 * Represents a frame sent over the the WebSocket connection.
 *
 * See also [RFC 6455, Section 5](http://tools.ietf.org/html/rfc6455#section-5).
 */
class QWsFrame
{
public:
  QWsFrame();


  /*!
   * Clears the payload
   */
  void clear();


  /*!
   * Performs various checks on the integrity of the frame as required by
   * RFC 6455
   */
  bool valid() const;

  /*!
   * Returns the unmaksed payload
   */
  QByteArray data() const;


  /*!
   * Returns true if the opcode is a control code
   */
  bool controlFrame() const;


  /*!
   * Tells us how many of the data fields have already been initialized
   */
  ReadingState readingState;

  bool final;
  quint8 rsv;
  bool hasMask;
  Opcode opcode;
  qint64 payloadLength;
  char maskingKey[4];
  QByteArray payload;
};

} // namespace QtWebsocket

#endif
