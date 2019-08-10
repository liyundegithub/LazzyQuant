#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <QString>

QString setupMessageHandler(bool toStdOut, bool toFile, const QString &moduleName, bool printTimeStamp = true);
void restoreMessageHandler();

#endif // MESSAGE_HANDLER_H
