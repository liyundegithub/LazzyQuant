#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <QtGlobal>

void toStdOut(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void toStdOutAndFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void toFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void setupMessageHandler(bool toStdOut, bool toFile, const QString &moduleName);
void restoreMessageHandler();

#endif // MESSAGE_HANDLER_H
