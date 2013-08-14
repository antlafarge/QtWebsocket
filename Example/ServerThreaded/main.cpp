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

#include "ServerThreaded.h"
#include <QtCore>
#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::cout << QObject::tr("main thread : 0x%1").arg(QString::number((unsigned int)QThread::currentThreadId(), 16)).toStdString() << std::endl;

	ServerThreaded myThreadedServer;

	return app.exec();
}
