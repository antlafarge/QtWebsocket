#include "Log.h"

Log * Log::sLog = 0;

Log * Log::getSingleton()
{
	if ( Log::sLog == 0 )
    {
		Log::sLog = new Log;
    }

	return Log::sLog;
}

void Log::display()
{
	Log::getSingleton();
	Log::sLog->show();
}

Log::Log() : QTextEdit()
{
	setWindowTitle(tr("Log"));
	//setTextInteractionFlags( Qt::TextBrowserInteraction );
	setReadOnly(true);
    setFixedSize(480, 640);

	connect( this, SIGNAL(newMessage(QString)), this, SLOT(appendToLog(QString)) );
}

Log::~Log()
{
	close();
}

void Log::closeEvent( QCloseEvent * e )
{
	QFile file(tr("Log.txt"));
	if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
    {
		return;
    }

	file.write( toPlainText().toUtf8() );

	file.close();
}

void Log::appendToLog(QString str)
{
	Log::getSingleton();

	if ( ! isVisible() )
    {
		show();
    }

	append(str);

	//repaint();
}

void Log::display(QString str)
{
	if ( Log::sLog == 0 )
    {
		return;
    }

	emit Log::sLog->newMessage(str);
}

void Log::display(QStringList strList)
{
	if ( Log::sLog == 0 )
    {
		return;
    }

	QString str;
	foreach (str, strList)
    {
		emit Log::sLog->newMessage(str);
    }
}

void Log::display(int val)
{
	if ( Log::sLog == 0 )
    {
		return;
    }

	emit Log::sLog->newMessage(QString::number(val));
}

void Log::display(float val)
{
    if ( Log::sLog == 0 )
    {
		return;
    }

	emit Log::sLog->newMessage(QString::number(val));
}
