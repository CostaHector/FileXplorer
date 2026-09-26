#ifndef MOVIEDBHELPER_H
#define MOVIEDBHELPER_H

#include <QString>

class QSqlRecord;
namespace MovieDbHelper {
QString GetWindowTitleName(const QSqlRecord& record);
QString GetDetailHtml(const QSqlRecord& record);
}

#endif // MOVIEDBHELPER_H
