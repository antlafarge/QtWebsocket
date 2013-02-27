#ifndef QWSFRAME_H
#define QWSFRAME_H

#include "QWsSocket.h"

#include <QByteArray>

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
  QByteArray unmask() const;


  /*!
   * Returns true if the opcode is a control code
   */
  bool controlFrame() const;


  /*!
   * Tells us how many of the data fields have already been initialized
   */
  QWsSocket::EReadingState readingState;

  bool final;
  quint8 rsv;
  bool hasMask;
  QWsSocket::EOpcode opcode;
  qint64 payloadLength;
  char maskingKey[4];
  QByteArray payload;
};

#endif
