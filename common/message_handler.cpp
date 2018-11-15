#include <stdio.h>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QCoreApplication>

#include "message_handler.h"

static FILE *pLogFile = NULL;

static void messageOut(QtMsgType type, const QMessageLogContext &context, const QByteArray &localMsg, FILE *pFile)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(pFile, "D: %s\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(pFile, "I: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(pFile, "W: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(pFile, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(pFile, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}

void toStdOut(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = (QDateTime::currentDateTime().toString(QStringLiteral("yy-MM-dd hh:mm:ss.zzz ")) + msg).toLocal8Bit();
    messageOut(type, context, localMsg, stdout);
    if (type == QtFatalMsg) {
        abort();
    }
}

void toStdOutAndFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = (QDateTime::currentDateTime().toString(QStringLiteral("yy-MM-dd hh:mm:ss.zzz ")) + msg).toLocal8Bit();
    messageOut(type, context, localMsg, stdout);
    messageOut(type, context, localMsg, pLogFile);
    if (type == QtFatalMsg) {
        fclose(pLogFile);
        abort();
    }
}

void toFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = (QDateTime::currentDateTime().toString(QStringLiteral("yy-MM-dd hh:mm:ss.zzz ")) + msg).toLocal8Bit();
    messageOut(type, context, localMsg, pLogFile);
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
