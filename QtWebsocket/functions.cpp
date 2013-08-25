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

#include <QtCore/qmath.h>

namespace QtWebsocket
{

quint8 bitCount(quint32 n)
{
	quint8 count = 0;
	while (n)
	{
		if (n & 1)
		{
			count++;
		}
		n >>= 1;
	}
	return count;
}

quint32 randquint32()
{
	const quint8 numberOfBits = bitCount(RAND_MAX);
	quint32 myRand = 0;
	int i = 3;
	while (i--)
	{
		myRand += qrand();
		myRand <<= numberOfBits;
	}
	return myRand;
}

quint32 randquint32(quint32 low, quint32 high)
{
	quint32 low2 = qMin(low, high);
	quint32 high2 = qMax(low, high);
	quint32 myRand = randquint32();
	double factor = (double)UINT_MAX / (double)(high2 - low2);
	return low2 + (myRand / factor);
}

} // namespace QtWebsocket
