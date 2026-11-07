#ifndef TABLENAMEHELPER_H
#define TABLENAMEHELPER_H

#include <QString>

namespace Base32Helper {
QString Encode(const QByteArray& data);
QByteArray Decode(const QString& input);
}

namespace TableNameHelper {
QString WorkPath2TableName(const QString& path);
QString TableName2WorkPath(const QString& tableName, bool& isValid);
};

#endif // TABLENAMEHELPER_H
