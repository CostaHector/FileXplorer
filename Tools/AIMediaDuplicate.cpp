#include "AIMediaDuplicate.h"
#include "public/PublicVariable.h"
#include "Tools/MD5Calculator.h"
#include <QDebug>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#ifdef _WIN32
#include "Tools/QMediaInfo.h"
#endif

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
  tableName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, "_");
  return tableName;
}

QString TableName2Path(const QString& tableName) {
  QString ans{tableName};
  if (ans.size() >= 3 && ans[1] == '_' && ans[2] == '_') {
    ans[1] = ':';
  }
  ans.replace('_', '/');
  return ans;
}

const char AIMediaDuplicate::CONNECTION_NAME[] = "RANDOM_CONNECT";
bool AIMediaDuplicate::SKIP_GETTER_DURATION = false;
bool AIMediaDuplicate::IS_TEST = false;

AIMediaDuplicate& AIMediaDuplicate::GetInst() {
  static AIMediaDuplicate inst;
  return inst;
}

AIMediaDuplicate::AIMediaDuplicate() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
  if (IS_TEST) {
    db.setDatabaseName("DUPLICATES_DB.db");
  } else {
    db.setDatabaseName(SystemPath::AI_MEDIA_DUP_DATABASE);
  }

  if (!db.open()) {
    qWarning("Open DB[%s] failed[%s]", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE), qPrintable(db.lastError().text()));
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
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return false;
  }
  return db.tables().contains(tableName);
}

int AIMediaDuplicate::ScanLocations(const QStringList& paths, bool dropFirst, bool skipWhenTableExist) {
  int succeedCnt = 0;
  for (const QString& path : paths) {
    if (!ScanALocation(path, dropFirst, skipWhenTableExist)) {
      qWarning("Scan Path[%s] failed", qPrintable(path));
      return false;
    }
    ++succeedCnt;
  }
  qDebug("%d/%d paths has been scanned succeed", succeedCnt, paths.size());
  return succeedCnt;
}

bool AIMediaDuplicate::ScanALocation(const QString& path, bool dropFirst, bool skipWhenTableExist) {
  if (!QFileInfo(path).isDir()) {
    qDebug("location[%s] is not a path", qPrintable(path));
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qDebug("DB[%s] not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return false;
  }
  QSqlQuery query(db);
  const QString& tableName = GetTableName(path);
  if (IsTableExist(tableName)) {
    if (skipWhenTableExist) {
      qDebug("Table[%s] already exists", qPrintable(tableName));
      return true;
    }
    if (dropFirst) {
      query.prepare(QString("DROP TABLE `%1`;").arg(tableName));
      if (!query.exec()) {
        qWarning("Drop table failed[%s]", qPrintable(tableName));
        return false;
      }
    }
  }

  query.prepare(QString("CREATE TABLE IF NOT EXISTS `%1` ("
                        "`EFFECTIVE_NAME` NCHAR(512) NOT NULL,"
                        "`SIZE` INTEGER NOT NULL,"
                        "`DURATION` INTEGER DEFAULT 0,"
                        "`DATE` INTEGER DEFAULT NULL,"
                        "`ABSOLUTE_PATH` NCHAR(512),"
                        "`FIRST_1024_HASH` NCHAR(32) DEFAULT NULL,"
                        "`FIRST_8192_HASH` NCHAR(32) DEFAULT NULL,"
                        "`FULL_SIZE_HASH` NCHAR(32) DEFAULT NULL,"
                        "PRIMARY KEY (`ABSOLUTE_PATH`, `SIZE`))")
                    .arg(tableName));
  if (!query.exec()) {
    qDebug("create table[%s] error[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
    return false;
  }
  query.prepare(QString("REPLACE INTO `%1` (EFFECTIVE_NAME, SIZE, DURATION, DATE, ABSOLUTE_PATH) VALUES (?, ?, ?, ?, ?)").arg(tableName));
  // 开始事务
  db.transaction();
#ifdef _WIN32
  QMediaInfo mi;
  if (!SKIP_GETTER_DURATION && !mi.StartToGet()) {
    qWarning("Video duration getter is nullptr");
    return false;
  }
#endif
  QDirIterator it(path, TYPE_FILTER::AI_DUP_VIDEO_TYPE_SET, QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QFileInfo file_info = it.next();
    const QString file_path = file_info.absoluteFilePath();
    query.bindValue(0, GetEffectiveName(file_path));
    query.bindValue(1, file_info.size());
#ifdef _WIN32
    query.bindValue(2, SKIP_GETTER_DURATION ? 0 : mi.VidDurationLengthQuick(file_path));
#else
    query.bindValue(2, 0);
#endif
    query.bindValue(3, file_info.birthTime().toMSecsSinceEpoch());
    query.bindValue(4, file_path);
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
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return -1;
  }
  int succeedDropCnt = 0;
  QSqlQuery query(db);
  const QStringList allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (!dropAll && !delTables.contains(tableName)) {
      continue;
    }
    if (!query.exec(QString("DROP TABLE `%1`;").arg(tableName))) {
      qWarning("Drop table[%s] failed. succeed cnt[%d]", qPrintable(tableName), succeedDropCnt);
      return succeedDropCnt;
    }
    ++succeedDropCnt;
    qDebug("Table[%s] already not exist in DB[%s]", qPrintable(tableName), qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
  }
  if (dropAll) {
    qDebug("drop table: [%d], succeed[%d]", allTables.size(), succeedDropCnt);
  } else {
    qDebug("drop table: [%d], succeed[%d]", delTables.size(), succeedDropCnt);
  }
  return succeedDropCnt;
}

int AIMediaDuplicate::AuditTables(const QStringList& atTables, bool auditAll) {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return -1;
  }
  int repairedCnt = 0;
  int auditTblCnt = 0;
  int ignoreCnt = 0;
  QSqlQuery query(db), delQry(db);
  for (const QString& tableName : db.tables()) {
    if (!auditAll && !atTables.contains(tableName)) {
      ++ignoreCnt;
      continue;
    }

    const QString& pth = TableName2Path(tableName);
    if (!QFileInfo(pth).isDir()) {
      qDebug("Driver[%s] of table[%s] is offline, skip audit", qPrintable(pth), qPrintable(tableName));
      continue;
    }
    query.prepare(QString("SELECT "
                          "       `ABSOLUTE_PATH` "
                          "FROM "
                          "	`%1`;")
                      .arg(tableName));
    if (!query.exec()) {
      qWarning("qry table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      continue;
    }
    delQry.prepare(QString("DELETE FROM `%1` "
                           "WHERE `ABSOLUTE_PATH` == (?)")
                       .arg(tableName));
    while (query.next()) {
      const QString& absPath = query.record().value(0).toString();
      if (QFile::exists(absPath)) {
        continue;
      }
      delQry.bindValue(0, absPath);
      if (!delQry.exec()) {
        qWarning("repair[%s] failed[%s]", qPrintable(absPath), qPrintable(delQry.lastError().text()));
        continue;
      }
      ++repairedCnt;
    }
    ++auditTblCnt;
  }
  qWarning("Audit repaird %d item(s) in %d table(s)", repairedCnt, auditTblCnt);
  return repairedCnt;
}

int AIMediaDuplicate::RebuildTables(const QStringList& rebTables, bool rebuildAll) {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return -1;
  }
  int succeedRebuildCnt = 0, ignoreCnt = 0;
  QSqlQuery query(db);
  const QStringList& allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (!rebuildAll && !rebTables.contains(tableName)) {
      ++ignoreCnt;
      continue;
    }
    const QString& path = TableName2Path(tableName);
    if (!ScanALocation(path, true, false)) {
      qWarning("rebuild table[%s] failed", qPrintable(tableName));
      continue;
    }
    ++succeedRebuildCnt;
  }
  return succeedRebuildCnt;
}

int AIMediaDuplicate::GetTablesCnt() const {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return -1;
  }
  return db.tables().size();
}

QHash<qint64, QString> AIMediaDuplicate::ReadATabel(const QString& tableName) {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  QSqlQuery query(db);
  query.exec(QString("SELECT `SIZE`, `EFFECTIVE_NAME` FROM `%1`").arg(tableName));
  QHash<qint64, QString> result;
  while (query.next()) {
    qint64 size = query.value(0).toLongLong();
    QString file_name = query.value(1).toString();
    result[size] = file_name;
  }
  return result;
}

int AIMediaDuplicate::FillHashFieldIfSizeConflict(const QString& path) {
  const QString& tableName = GetTableName(path);
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return -1;
  }

  if (!db.tables().contains(tableName)) {
    qDebug("table[%s] not exist no need to fill hash Field", qPrintable(tableName));
    return 0;
  }

  QSqlQuery query(db);
  query.prepare(QString("SELECT "
                        "       `ABSOLUTE_PATH`,"
                        "	`SIZE` "
                        "FROM "
                        "	`%1` "
                        "WHERE "
                        "       `FIRST_1024_HASH` IS NULL;")
                    .arg(tableName));
  if (!query.exec()) {
    qWarning("qry error[%s]", qPrintable(query.lastError().text()));
    return -1;
  }
  QHash<qint64, QStringList> size2absPath;
  while (query.next()) {
    const auto& rec = query.record();
    size2absPath[rec.value(1).toLongLong()] << rec.value(0).toString();
  }
  query.finish();

  QSqlQuery setHash(db);
  setHash.prepare(QString("UPDATE `%1`"
                          "SET `FIRST_1024_HASH` = (?)"
                          "WHERE `ABSOLUTE_PATH` == (?);")
                      .arg(tableName));

  using namespace MD5Calculator;
  int cnt = 0;
  for (auto i = size2absPath.cbegin(), end = size2absPath.cend(); i != end; ++i) {
    const QStringList& pths = i.value();
    if (pths.size() < 2) {
      continue;
    }
    for (const QString& abspath : pths) {
      setHash.bindValue(0, GetFileMD5(abspath, 1024));
      setHash.bindValue(1, abspath);
      if (!setHash.exec()) {
        qWarning("set field md5 of[`%s`] failed", qPrintable(abspath));
        continue;
      }
      ++cnt;
    }
  }
  qDebug("Fill %d record(s) on hash fields", cnt);
  return cnt;
}

int AIMediaDuplicate::ReadSpecifiedTables2List(const QStringList& tbls, QList<DUP_INFO>& vidsInfo) {
  if (tbls.isEmpty()) {
    vidsInfo.clear();
    return 0;
  }
  const int beforeCnt = vidsInfo.size();
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    return -1;
  }
  int succeedTblCnt = 0;
  QSqlQuery query(db);
  for (const QString& tableName : tbls) {
    if (!query.exec(QString("SELECT `EFFECTIVE_NAME`, `SIZE`, `DURATION`, `DATE`, `ABSOLUTE_PATH`, `FIRST_1024_HASH` FROM `%1`;").arg(tableName))) {
      qWarning("qry table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      return succeedTblCnt;
    }
    while (query.next()) {
      vidsInfo.append(DUP_INFO{query.value(0).toString(), query.value(1).toLongLong(), query.value(2).toInt(), query.value(3).toLongLong(),
                               query.value(4).toString(), query.value(5).toString(), true});
    }
    ++succeedTblCnt;
  }
  qDebug("before %d row(s), now %d row(s), succeed %d table(s)", beforeCnt, vidsInfo.size(), succeedTblCnt);
  return true;
}

QList<DupTableModelData> AIMediaDuplicate::TableName2Cnt() {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    qWarning("DB[%s] is not open", qPrintable(SystemPath::AI_MEDIA_DUP_DATABASE));
    return {};
  }
  QList<DupTableModelData> tbl2Cnt;
  QSqlQuery query(db);
  for (const QString& tableName : db.tables()) {
    if (!query.exec(QString("SELECT COUNT(`EFFECTIVE_NAME`) FROM `%1`;").arg(tableName))) {
      qDebug("count table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      continue;
    }
    int recordCnt = 0;
    while (query.next()) {
      recordCnt = query.record().value(0).toInt();
    }
    tbl2Cnt.append({tableName, recordCnt});  // query.record().count();
  }
  qDebug("Update Tables Count:%d", tbl2Cnt.size());
  return tbl2Cnt;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& aid = AIMediaDuplicate::GetInst();
  aid.ScanALocation("E:/P/Hetero", true, false);
  aid.FillHashFieldIfSizeConflict("E:/P/Hetero");
  return 0;
}

#endif
