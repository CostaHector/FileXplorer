#include "MovieBaseDb.h"
#include "public/PublicVariable.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "public/PublicMacro.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDirIterator>
#include <QStorageInfo>
#include <QDateTime>

using namespace DB_HEADER_KEY;

const QString MovieBaseDb::CREATE_MOVIE_TABLE_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1` ("                //
     + QString("   `%1` NCHAR(256) NOT NULL,"           // Name
               "   `%2` INT DEFAULT 0,"                 // Size
               "   `%3` CHAR(10),"                      // Type
               "   `%4` CHAR(20),"                      // DateModified
               "   `%5` NCHAR(40),"                     // Performers
               "   `%6` CHAR(40),"                      // Tags
               "   `%7` INT DEFAULT 0,"                 // Rate
               "   `%8` CHAR(20),"                      // Driver
               "   `%9` NCHAR(256) NOT NULL,"           // Prepath
               "   `%10` CHAR(128),"                    // Extra
               "   `%11` NCHAR(512) NOT NULL,"          // DB_HEADER_KEY::ForSearch
               "    PRIMARY KEY (%1, %9, %11)"
               "    );")
           .arg(VOLUME_ENUM_TO_STRING(Name))
           .arg(VOLUME_ENUM_TO_STRING(Size))
           .arg(VOLUME_ENUM_TO_STRING(Type))
           .arg(VOLUME_ENUM_TO_STRING(DateModified))
           .arg(VOLUME_ENUM_TO_STRING(Performers))
           .arg(VOLUME_ENUM_TO_STRING(Tags))
           .arg(VOLUME_ENUM_TO_STRING(Rate))
           .arg(VOLUME_ENUM_TO_STRING(Driver))
           .arg(VOLUME_ENUM_TO_STRING(Prepath))
           .arg(VOLUME_ENUM_TO_STRING(Extra))
           .arg(VOLUME_ENUM_TO_STRING(ForSearch))};

const QString MovieBaseDb::INSERT_MOVIE_RECORD_TEMPLATE  //
    {"REPLACE INTO `%1` "                                //
     + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`, `%8`, `%9`, `%10`, `%11`)
VALUES(:%1, :%2, :%3, :%4, :%5, :%6, :%7, :%8, :%9, :%10, :%11);)")
           .arg(VOLUME_ENUM_TO_STRING(Name))
           .arg(VOLUME_ENUM_TO_STRING(Size))
           .arg(VOLUME_ENUM_TO_STRING(Type))
           .arg(VOLUME_ENUM_TO_STRING(DateModified))
           .arg(VOLUME_ENUM_TO_STRING(Performers))
           .arg(VOLUME_ENUM_TO_STRING(Tags))
           .arg(VOLUME_ENUM_TO_STRING(Rate))
           .arg(VOLUME_ENUM_TO_STRING(Driver))
           .arg(VOLUME_ENUM_TO_STRING(Prepath))
           .arg(VOLUME_ENUM_TO_STRING(Extra))
           .arg(VOLUME_ENUM_TO_STRING(ForSearch))};

int MovieBaseDb::ReadVideosFromAPath(const QString& path, const QString& tableName) {
  if (!QFileInfo(path).isDir()) {
    qWarning("folderAbsPath[%s] is not a directory", qPrintable(path));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  QSqlQuery query(db);
  if (!query.prepare(INSERT_MOVIE_RECORD_TEMPLATE.arg(tableName))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  // 开始事务
  if (!db.transaction()) {
    qWarning("start the %dth transaction failed: %s",  //
             1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  const QString CURRENT_DRIVE_LETTER = QStorageInfo(path).rootPath();

  QDirIterator it{path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Files, QDirIterator::Subdirectories};
  int count = 0;
  while (it.hasNext()) {
    const QString& absFilePath = it.next();
    const QFileInfo fileInfo(absFilePath);
    const QFileInfo& fi = it.fileInfo();
    query.bindValue(DB_HEADER_KEY::Name, fi.fileName());
    query.bindValue(DB_HEADER_KEY::Size, fi.size());
    query.bindValue(DB_HEADER_KEY::Type, fi.suffix());
    query.bindValue(DB_HEADER_KEY::DateModified, fi.lastModified().toString("yyyy/MM/dd HH:mm:ss"));
    query.bindValue(DB_HEADER_KEY::Performers, "");
    query.bindValue(DB_HEADER_KEY::Tags, "");
    query.bindValue(DB_HEADER_KEY::Rate, 0);
    query.bindValue(DB_HEADER_KEY::Driver, CURRENT_DRIVE_LETTER);
    query.bindValue(DB_HEADER_KEY::Prepath, fi.absolutePath());
    query.bindValue(DB_HEADER_KEY::Extra, "");
    query.bindValue(DB_HEADER_KEY::ForSearch, fi.absoluteFilePath());

    if (!query.exec()) {
      db.rollback();
      qWarning("replace[%s] failed: %s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return -1;
    }

    count++;

    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        qWarning("commit the %dth batch record(s) failed: %s",  //
                 count / MAX_BATCH_SIZE + 1, qPrintable(db.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        qWarning("start the %dth transaction failed: %s",  //
                 count / MAX_BATCH_SIZE + 2, qPrintable(db.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    qWarning("remain record(s) commit failed: %s", qPrintable(db.lastError().text()));
    return FD_COMMIT_FAILED;
  }
  query.finish();
  qWarning("%d record(s) commit replaced into succeed", count);
  return count;
}
