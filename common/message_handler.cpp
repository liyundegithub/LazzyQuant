#include <stdio.h>
#include <QByteArray>
#include <QDateTime>
#include <QCoreApplication>

#include "message_handler.h"

static FILE *pLogFile = NULL;

static inline QByteArray getLocalMsg(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString typeHeader = "";
    switch (type) {
    case QtDebugMsg:
        typeHeader = "D";
        break;
    case QtInfoMsg:
        typeHeader = "I";
        break;
    case QtWarningMsg:
        typeHeader = "W";
        break;
    case QtCriticalMsg:
        typeHeader = "C";
        break;
    case QtFatalMsg:
        typeHeader = "F";
        break;
    }
    QString localMsg = typeHeader + QDateTime::currentDateTime().toString(QStringLiteral(": yy-MM-dd hh:mm:ss.zzz ")) + msg;
#ifdef QT_MESSAGELOGCONTEXT
    localMsg += QString(" (%1:%2, %3)").arg(context.file).arg(context.line).arg(context.function);
#endif
    return localMsg.toLocal8Bit();
}

static void toStdOut(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = getLocalMsg(type, context, msg);
    fprintf(stdout, "%s\n", localMsg.constData());
    if (type == QtFatalMsg) {
        abort();
    }
}

static void toStdOutAndFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = getLocalMsg(type, context, msg);
    fprintf(stdout, "%s\n", localMsg.constData());
    fprintf(pLogFile, "%s\n", localMsg.constData());
    if (type == QtFatalMsg) {
        fclose(pLogFile);
        abort();
    }
}

static void toFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = getLocalMsg(type, context, msg);
    fprintf(pLogFile, "%s\n", localMsg.constData());
    if (type == QtFatalMsg) {
        fclose(pLogFile);
        abort();
    }
}

QString setupMessageHandler(bool logtoStdout, bool logtoFile, const QString &moduleName)
{
    if (logtoFile) {
        QString fullFileName = QCoreApplication::applicationDirPath() + "/" + moduleName;
        fullFileName += QDateTime::currentDateTime().toString(QStringLiteral("_yyyyMMdd_hhmmss"));
        fullFileName.append(".txt");
        pLogFile = fopen(qPrintable(fullFileName), "a");
        if (pLogFile) {
            if (logtoStdout) {
                qInstallMessageHandler(toStdOutAndFile);
            } else {
                qInstallMessageHandler(toFile);
            }
        }
        return fullFileName;
    } else {
        if (logtoStdout) {
            qInstallMessageHandler(toStdOut);
        } else {
            // Do nothing
        }
        return QString();
    }
}

void restoreMessageHandler()
{
    qInstallMessageHandler(0);
    if (pLogFile) {
        fclose(pLogFile);
        pLogFile = NULL;
    }
}
