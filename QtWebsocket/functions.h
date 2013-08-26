/*
Copyright (C) 2013 Antoine Lafarge qtwebsocket@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QTWS_FUNCTIONS_H
#define QTWS_FUNCTIONS_H

#include <QtCore/qmath.h>

namespace QtWebsocket
{

bool rand2();
quint8 rand8(quint8 low = 0, quint8 high = 0);
quint16 rand16(quint16 low = 0, quint16 high = 0);
quint32 rand32(quint32 low = 0, quint32 high = 0);
quint64 rand64(quint64 low = 0, quint64 high = 0);

} // namespace QtWebsocket

#endif // QTWS_FUNCTIONS_H
