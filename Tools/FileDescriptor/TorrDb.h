#ifndef TORRDB_H
#define TORRDB_H

#include <QString>
#include "DbManager.h"
class TorrDb : public DbManager {
 public:
  TorrDb(const QString& dbName, const QString& connName, QObject* parent = nullptr);
  static const QString CREATE_TABLE_TEMPLATE;
  static const QString REPLACE_INTO_TABLE_TEMPLATE;
};

#endif  // TORRDB_H
