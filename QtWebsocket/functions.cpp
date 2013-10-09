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

#include <QtCore/qmath.h>
#include <climits>

namespace QtWebsocket
{

bool rand2()
{
	return (qrand() & 0x1) == 0x1;
}

quint8 rand8(quint8 low, quint8 high)
{
	if (low == 0 && high == 0)
	{
		high = UCHAR_MAX;
	}
	else if (low > high)
	{
		qSwap(low, high);
	}
	return low + (qrand() % (high - low + 1));
}

quint16 rand16(quint16 low, quint16 high)
{
	if (low == 0 && high == 0)
	{
		high = USHRT_MAX;
	}
	else if (low > high)
	{
		qSwap(low, high);
	}
	quint16 range = high - low;
	if (range < RAND_MAX)
	{
		return low + (qrand() % (range + 1));
	}
	else
	{
		quint16 myRand = qrand();
		myRand += (((quint16)qrand()) << 15);
		return low + (myRand % (high - low + 1));
	}
}

quint32 rand32(quint32 low, quint32 high)
{
	if (low == 0 && high == 0)
	{
		high = ULONG_MAX;
	}
	else if (low > high)
	{
		qSwap(low, high);
	}
	quint32 myRand = qrand();
	myRand += (((quint32)qrand()) << 15);
	myRand += (((quint32)qrand()) << 30);
	return low + (myRand % (high - low + 1));
}

quint64 rand64(quint64 low, quint64 high)
{
	if (low == 0 && high == 0)
	{
		high = ULLONG_MAX;
	}
	else if (low > high)
	{
		qSwap(low, high);
	}
	quint64 myRand = qrand();
	myRand += (((quint64)qrand()) << 15);
	myRand += (((quint64)qrand()) << 30);
	myRand += (((quint64)qrand()) << 45);
	myRand += (((quint64)qrand()) << 60);
	return low + (myRand % (high - low + 1));
}

} // namespace QtWebsocket
