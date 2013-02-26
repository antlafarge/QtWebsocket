#ifndef QWSFRAME_H
#define QWSFRAME_H

#include "QWsSocket.h"

#include <QByteArray>

class QWsFrame
{

public:

  QWsFrame();
  void clear();
  QByteArray data() const; // TODO make const
  bool valid();
  QByteArray unmask() const;

  bool final;
  quint8 rsv;
  bool hasMask;
  QWsSocket::EOpcode opcode;
  quint64 payloadLength;
  QByteArray maskingKey; // TODO convert to quint8
  QByteArray payload;
  QWsSocket::EReadingState readingState;
};

#endif
