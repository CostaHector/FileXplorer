#ifndef MOVIEBASEDB_H
#define MOVIEBASEDB_H

#include "DbManager.h"

class MovieBaseDb : public DbManager {
 public:
  MovieBaseDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {                                             //
  }
  int ReadVideosFromAPath(const QString& path, const QString& tableName);
  static const QString CREATE_MOVIE_TABLE_TEMPLATE;
  static const QString INSERT_MOVIE_RECORD_TEMPLATE;
};

#endif  // MOVIEBASEDB_H
