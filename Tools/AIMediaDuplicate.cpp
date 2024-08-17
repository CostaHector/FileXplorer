#include "AIMediaDuplicate.h"
#include "PublicVariable.h"
#include <QDebug>
#include <QDirIterator>
#include <QSqlError>

QString GetEffectiveName(const QString& itemPath) {
  static const QSet<QString> specialSegments{"videos", "video", "vid", "video_ts"};
  const QString segment = itemPath.section('/', -2, -2).toLower();
  if (specialSegments.contains(segment)) {
    return itemPath.section('/', -3, -1);
  }
  return itemPath.section('/', -2, -1);
}

QString GetTableName(const QString& pathName) {
  QString tableName{pathName};
  if (pathName.size() > 3 && pathName[1] == ':') {
    tableName = pathName.mid(3);
  }
  tableName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, "_");
  return tableName;
}

const char AIMediaDuplicate::DB[] = "DUPLICATES_DB.db";
const char AIMediaDuplicate::CONNECTION_NAME[] = "RANDOM_CONNECT";

AIMediaDuplicate& AIMediaDuplicate::GetInst() {
  static AIMediaDuplicate inst;
  return inst;
}

AIMediaDuplicate::AIMediaDuplicate() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
  db.setDatabaseName(DB);
  if (!db.open()) {
    qWarning("Open DB[%s] failed[%s]", DB, qPrintable(db.lastError().text()));
  }
}

AIMediaDuplicate::~AIMediaDuplicate() {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (db.isOpen()) {
    db.close();
  }
}

bool AIMediaDuplicate::IsTableExist(const QString& tableName) const {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", DB);
    return false;
  }
  return db.tables().contains(tableName);
}

int AIMediaDuplicate::ScanLocations(const QStringList& paths, bool eraseFirst, bool skipWhenItemExist) {
  int succeedCnt = 0;
  for (const QString& path : paths) {
    if (!ScanALocation(path, eraseFirst, skipWhenItemExist)) {
      qWarning("Scan Path[%s] failed", qPrintable(path));
      return false;
    }
    ++succeedCnt;
  }
  qDebug("%d/%d paths has been scanned succeed", succeedCnt, paths.size());
  return succeedCnt;
}

bool AIMediaDuplicate::ScanALocation(const QString& path, bool eraseFirst, bool skipWhenItemExist) {
  if (!QFileInfo(path).isDir()) {
    qDebug("location[%s] is not a path", qPrintable(path));
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  //  db.setDatabaseName(DB);
  if (!db.isOpen()) {
    qDebug("DB[%s] not open", DB);
    return false;
  }
  QSqlQuery query(db);
  const QString& tableName = GetTableName(path);
  if (eraseFirst && IsTableExist(tableName)) {
    if (!query.exec(QString("DROP TABLE `%1`;").arg(tableName))) {
      qWarning("Drop table failed[%s]", qPrintable(tableName));
      return false;
    }
  }

  if (!query.exec(QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`EFFECTIVE_NAME` NCHAR(512) NOT NULL,"
                          "`SIZE` INTEGER,"
                          "`DURATION` INTEGER,"
                          "`ABSOLUTE_PATH` NCHAR(512),"
                          "PRIMARY KEY (`ABSOLUTE_PATH`))")
                      .arg(tableName))) {
    qDebug("create table[%s] error[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
    return false;
  }

  // 开始事务
  db.transaction();

  const QString REPLACE_CMD = QString("REPLACE INTO `%1` (EFFECTIVE_NAME, SIZE, DURATION, ABSOLUTE_PATH) VALUES (?, ?, ?, ?)").arg(tableName);

  QDirIterator it(path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QFileInfo file_info = it.next();
    const QString file_path = file_info.absoluteFilePath();
    int duration = 0;  // 文件时长固定为0
    // 插入表TABLE
    query.prepare(REPLACE_CMD);
    query.bindValue(0, GetEffectiveName(file_path));
    query.bindValue(1, file_info.size());
    query.bindValue(2, duration);
    query.bindValue(3, file_path);
    if (!query.exec()) {
      qDebug("insert error[%s]", qPrintable(query.lastError().text()));
    }
  }
  query.clear();
  db.commit();
  return true;
}

int AIMediaDuplicate::DropTables(const QStringList& delTables, bool dropAll) {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", DB);
    return -1;
  }
  int succeedDropCnt = 0, notExistsCnt = 0;
  QSqlQuery query(db);
  const QStringList allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (dropAll || delTables.contains(tableName)) {
      if (!query.exec(QString("DROP TABLE `%1`;").arg(tableName))) {
        qWarning("Drop table[%s] failed. succeed cnt[%d]", qPrintable(tableName), succeedDropCnt);
        return succeedDropCnt;
      }
      ++succeedDropCnt;
      continue;
    }
    qDebug("Table[%s] already not exist in DB[%s]", qPrintable(tableName), DB);
    ++notExistsCnt;
  }
  if (dropAll) {
    qDebug("drop table: [%d], succeed[%d], ignore[%d]", allTables.size(), succeedDropCnt, notExistsCnt);
  } else {
    qDebug("drop table: [%d], succeed[%d], ignore[%d]", delTables.size(), succeedDropCnt, notExistsCnt);
  }
  return succeedDropCnt;
}

int AIMediaDuplicate::GetTablesCnt() const {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", DB);
    return -1;
  }
  return db.tables().size();
}

QHash<qint64, QString> AIMediaDuplicate::ReadATabel(const QString& tabelName) {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  QSqlQuery query(db);
  query.exec(QString("SELECT SIZE, EFFECTIVE_NAME FROM `%1`").arg(tabelName));
  QHash<qint64, QString> result;
  while (query.next()) {
    qint64 size = query.value(0).toLongLong();
    QString file_name = query.value(1).toString();
    result[size] = file_name;
  }
  return result;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  AIMediaDuplicate aid;
  aid.ScanALocation("E:/P/Hetero", true);
  return 0;
}

#endif
