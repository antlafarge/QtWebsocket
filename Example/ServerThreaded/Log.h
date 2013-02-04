#ifndef Log_H
#define Log_H

#include <QtCore>
#include <QtGui>

class Log : public QTextEdit
{
	Q_OBJECT

public:
	static Log * getSingleton();
	static void display();

	static void display(QString str);
	static void display(QStringList strList);
	static void display(int val);
	static void display(float val);

	void closeEvent( QCloseEvent * event );

public slots:
	void appendToLog(QString str);

    // XXX: this command for checking that eash connection lives in own thread
    // each client will see own messages
    void blockApp();

signals:
	// On utilise un signal pour changer de thread
	// (QtGui est dans le "main thread", Le slot appendToLog sera donc éxecutée par le bon thread)
    void newMessage(QString str);

protected:
	Log();
	~Log();

	static Log * sLog;
};

#endif // Log_H
