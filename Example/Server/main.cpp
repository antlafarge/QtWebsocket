#include "Server.h"
#include "Log.h"
#include <QApplication>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString msg2;
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        msg2 = QString("Debug: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
        //fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        msg2 = QString("Warning: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
        //fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        msg2 = QString("Critical: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
        //fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        msg2 = QString("Fatal: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
        //fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }

    Log::display(msg2);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QApplication a(argc, argv);

    Log::display();

    Server myServer;

    return a.exec();
}
