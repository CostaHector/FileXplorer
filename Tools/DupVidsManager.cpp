#include "DupVidsManager.h"
#include "PublicVariable.h"
#include "JsonRenameRegex.h"
#include "MD5Calculator.h"
#include "VideoDurationGetter.h"
#include "Logger.h"
#include "PathTool.h"

#include <QDebug>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QSqlQuery>
#include <QUuid>

QString GetTableName(const QString& pathName) {
  QString tableName{pathName};
  tableName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, "_");  // onlt support path with [0-9a-z_]
  return tableName;
}

QString TableName2Path(const QString& tableName) {
  QString ans{tableName};
#ifdef _WIN32
  if (ans.size() >= 3 && ans[1] == '_' && ans[2] == '_') {
    ans[1] = ':';
  }
#endif
  ans.replace('_', '/');
  return ans;
}

constexpr char DupVidsManager::CONNECTION_NAME[];

#include "TableFields.h"
#include "PublicMacro.h"

using namespace VidDupHelper;

const QString DupVidsManager::CREATE_DUP_VID_TABLE_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1`"                         //
     + QString(R"((
      `%1` NCHAR(512) NOT NULL,
      `%2` INTEGER NOT NULL,
      `%3` INTEGER DEFAULT 0,
      `%4` INTEGER DEFAULT NULL,
      `%5` NCHAR(512),
      `%6` NCHAR(32) DEFAULT NULL,
      `%7` NCHAR(32) DEFAULT NULL,
      `%8` NCHAR(32) DEFAULT NULL,
      PRIMARY KEY (`%5`, `%2`)
);)")
           .arg(ENUM_2_STR(EFFECTIVE_NAME))
           .arg(ENUM_2_STR(SIZE))
           .arg(ENUM_2_STR(DURATION))
           .arg(ENUM_2_STR(DATE))
           .arg(ENUM_2_STR(ABSOLUTE_PATH))
           .arg(ENUM_2_STR(FIRST_1024_HASH))
           .arg(ENUM_2_STR(FIRST_8192_HASH))
           .arg(ENUM_2_STR(FULL_SIZE_HASH))};

DupVidsManager::DupVidsManager(const QString& dbName, const QString& connName, QObject* parent)  //
    : DbManager{dbName, connName, parent} {}

DupVidsManager& DupVidsManager::GetInst() {
  static DupVidsManager inst{DupVidsManager::GetAiDupVidDbPath(), CONNECTION_NAME, nullptr};
  return inst;
}

QString DupVidsManager::GetAiDupVidDbPath() {
#ifdef RUNNING_UNIT_TESTS
  return TESTCASE_ROOT_PATH "/test/DUPLICATES_DB_TEST.db";
#else
  return SystemPath::AI_MEDIA_DUP_DATABASE;
#endif
}

bool DupVidsManager::IsTableExist(const QString& tableName) const {
  QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
  if (!db.isOpen()) {
    LOG_W("DB[%s] is not open", qPrintable(GetAiDupVidDbPath()));
    return false;
  }
  return db.tables().contains(tableName);
}

int DupVidsManager::ScanLocations(const QStringList& paths) {
  int succeedCnt = 0;
  for (const QString& path : paths) {
    if (!ScanALocation(path)) {
      LOG_W("Scan Path[%s] failed", qPrintable(path));
      return false;
    }
    ++succeedCnt;
  }
  LOG_D("%d/%d paths has been scanned succeed", succeedCnt, paths.size());
  return succeedCnt;
}

const QString DupVidsManager::INSERT_DUP_VID_TEMPLATE  //
    {"REPLACE INTO `%1` "                                //
     + QString{R"((`%1`,`%2`,`%3`,`%4`,`%5`) VALUES(:1, :2, :3, :4, :5);)"}
           .arg(ENUM_2_STR(EFFECTIVE_NAME))
           .arg(ENUM_2_STR(SIZE))
           .arg(ENUM_2_STR(DURATION))
           .arg(ENUM_2_STR(DATE))
           .arg(ENUM_2_STR(ABSOLUTE_PATH))};

enum INSERT_DUP_VID_TEMPLATE_FIELD {  //  DO UPDATE SET `%2`=:%3, `%3`=:%4; must!
  INSERT_DUP_VID_TEMPLATE_FIELD_EFFECTIVE_NAME = 0,
  INSERT_DUP_VID_TEMPLATE_FIELD_SIZE,
  INSERT_DUP_VID_TEMPLATE_FIELD_DURATION,
  INSERT_DUP_VID_TEMPLATE_FIELD_DATE,
  INSERT_DUP_VID_TEMPLATE_FIELD_ABSOLUTE_PATH,
};

bool DupVidsManager::ScanALocation(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    LOG_D("location[%s] is not a path", qPrintable(path));
    return false;
  }
  const QString& tableName = GetTableName(path);
  if (!CreateTable(tableName, CREATE_DUP_VID_TABLE_TEMPLATE)) {
    LOG_W("Table[%s] create failed.", qPrintable(tableName));
    return false;
  }
  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    LOG_W("Video duration getter start failed");
    return false;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s", 1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  QSqlQuery query{db};
  if (!query.prepare(INSERT_DUP_VID_TEMPLATE.arg(tableName))) {
    return FD_PREPARE_FAILED;
  }
  int suceedCnt = 0;
  QDirIterator it{path, TYPE_FILTER::AI_DUP_VIDEO_TYPE_SET, QDir::Files, QDirIterator::Subdirectories};
  while (it.hasNext()) {
    const QString file_path = it.next();
    const QFileInfo file_info{file_path};
    query.bindValue(INSERT_DUP_VID_TEMPLATE_FIELD_EFFECTIVE_NAME, PathTool::GetEffectiveName(file_path));
    query.bindValue(INSERT_DUP_VID_TEMPLATE_FIELD_SIZE, file_info.size());
    query.bindValue(INSERT_DUP_VID_TEMPLATE_FIELD_DURATION, mi.GetLengthQuick(file_path));
    query.bindValue(INSERT_DUP_VID_TEMPLATE_FIELD_DATE, file_info.birthTime().toMSecsSinceEpoch());
    query.bindValue(INSERT_DUP_VID_TEMPLATE_FIELD_ABSOLUTE_PATH, file_path);
    if (!query.exec()) {
      LOG_W("insert error[%s]", qPrintable(query.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++suceedCnt;
  }
  query.finish();
  bool commitResult = db.commit();
  LOG_D("%d records(s) submit bResult: %d", suceedCnt, commitResult);
  return commitResult;
}

int DupVidsManager::DropTables(const QStringList& delTables) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  const QStringList allTables = db.tables();
  const QSet<QString> allTablesSet{allTables.begin(), allTables.end()};
  int succeedDropCnt = 0;
  for (const QString& toDel : delTables) {
    if (!allTablesSet.contains(toDel)) {
      continue;
    }
    succeedDropCnt += (RmvTable(toDel, DROP_OR_DELETE::DROP, true) >= FD_OK);
  }
  LOG_D("drop %d/%d/%d table succeed", succeedDropCnt, delTables.size(), allTablesSet.size());
  return succeedDropCnt;
}

int DupVidsManager::AuditTables(const QStringList& atTables, bool auditAll) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
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
      LOG_D("Driver[%s] of table[%s] is offline, skip audit", qPrintable(pth), qPrintable(tableName));
      continue;
    }
    query.prepare(QString("SELECT `ABSOLUTE_PATH` FROM `%1`;").arg(tableName));
    if (!query.exec()) {
      LOG_W("qry table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      continue;
    }
    delQry.prepare(QString("DELETE FROM `%1` WHERE `ABSOLUTE_PATH` == (?)").arg(tableName));
    while (query.next()) {
      const QString& absPath = query.record().value(0).toString();
      if (QFile::exists(absPath)) {
        continue;
      }
      delQry.bindValue(0, absPath);
      if (!delQry.exec()) {
        LOG_W("repair[%s] failed[%s]", qPrintable(absPath), qPrintable(delQry.lastError().text()));
        continue;
      }
      ++repairedCnt;
    }
    ++auditTblCnt;
  }
  LOG_W("Audit repaird %d item(s) in %d table(s)", repairedCnt, auditTblCnt);
  return repairedCnt;
}

int DupVidsManager::RebuildTables(const QStringList& rebTables) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  int succeedRebuildCnt = 0;
  const QStringList& allTables = db.tables();
  const QStringList& allTablesSet{allTables.begin(), allTables.end()};
  for (const QString& tableName : rebTables) {
    if (allTablesSet.contains(tableName)) {
      continue;
    }
    const QString& path = TableName2Path(tableName);
    if (!ScanALocation(path)) {
      LOG_W("rebuild table[%s] failed", qPrintable(tableName));
      continue;
    }
    ++succeedRebuildCnt;
  }
  LOG_D("%d/%d/%d rebuild", succeedRebuildCnt, rebTables.size(), allTablesSet.size());
  return succeedRebuildCnt;
}

int DupVidsManager::GetTablesCnt() const {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }
  const QStringList tbs = db.tables();
  return tbs.size();
}

const QString DupVidsManager::READ_DUP_VID_TABLE_META_INFO  //
    {
        QString("SELECT `%1`, `%2` FROM ")  //
            .arg(ENUM_2_STR(SIZE))          //
            .arg(ENUM_2_STR(EFFECTIVE_NAME)) +
        "`%1`"  //
    };
enum READ_DUP_VID_TABLE_META_INFO_FIELD {  //  DO UPDATE SET `%2`=:%3, `%3`=:%4; must!
  READ_DUP_VID_TABLE_META_INFO_FIELD_SIZE = 0,
  READ_DUP_VID_TABLE_META_INFO_FIELD_EFFECTIVE_NAME = 1,
};

QHash<qint64, QString> DupVidsManager::ReadATable(const QString& tableName) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Read a table meta info failed");
    return {};
  }
  QSqlQuery query(db);
  query.exec(READ_DUP_VID_TABLE_META_INFO.arg(tableName));
  QHash<qint64, QString> result;
  while (query.next()) {
    qint64 size = query.value(READ_DUP_VID_TABLE_META_INFO_FIELD_SIZE).toLongLong();
    QString file_name = query.value(READ_DUP_VID_TABLE_META_INFO_FIELD_EFFECTIVE_NAME).toString();
    result[size] = file_name;
  }
  return result;
}

const QString DupVidsManager::FIND_SAME_SIZE_VID  //
    {QString("SELECT `%1`, `%2` FROM ")             //
         .arg(ENUM_2_STR(ABSOLUTE_PATH))            //
         .arg(ENUM_2_STR(SIZE)) +
     QString{" `%1` "}                  //
     + QString(" WHERE `%1` IS NULL;")  //
           .arg(ENUM_2_STR(FIRST_1024_HASH))};

enum FIND_SAME_SIZE_VID_FIELD {                //
  FIND_SAME_SIZE_VID_FIELD_ABSOLUTE_PATH = 0,  //
  FIND_SAME_SIZE_VID_FIELD_SIZE,               //
  FIND_SAME_SIZE_VID_FIELD_FIRST_1024_HASH     //
};

const QString DupVidsManager::UPDATE_HASH_BY_PRIMARY_KEY  //
    {
        QString{"UPDATE `%1` "}  //
        + QString(" SET `%1` = (:1) WHERE `%2` == (:2);")
              .arg(ENUM_2_STR(FIRST_1024_HASH))  //
              .arg(ENUM_2_STR(ABSOLUTE_PATH))    //
    };

enum UPDATE_HASH_BY_PRIMARY_KEY_FIELD {
  UPDATE_HASH_BY_PRIMARY_KEY_FIELD_FIRST_1024_HASH = 0,
  UPDATE_HASH_BY_PRIMARY_KEY_FIELD_ABSOLUTE_PATH

};

int DupVidsManager::FillHashFieldIfSizeConflict(const QString& path) {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  const QString& tableName = GetTableName(path);
  if (!db.tables().contains(tableName)) {
    LOG_D("table[%s] not exist no need to fill hash Field", qPrintable(tableName));
    return 0;
  }

  QSqlQuery query(db);
  query.prepare(FIND_SAME_SIZE_VID.arg(tableName));
  if (!query.exec()) {
    LOG_W("qry error[%s]", qPrintable(query.lastError().text()));
    return -1;
  }
  QHash<qint64, QStringList> size2absPath;
  while (query.next()) {
    size2absPath[query.value(FIND_SAME_SIZE_VID_FIELD_SIZE).toLongLong()] << query.value(FIND_SAME_SIZE_VID_FIELD_ABSOLUTE_PATH).toString();
  }
  query.finish();

  QSqlQuery setHash(db);
  setHash.prepare(UPDATE_HASH_BY_PRIMARY_KEY.arg(tableName));

  using namespace MD5Calculator;
  int cnt = 0;
  for (auto i = size2absPath.cbegin(), end = size2absPath.cend(); i != end; ++i) {
    const QStringList& pths = i.value();
    if (pths.size() < 2) {
      continue;
    }
    for (const QString& abspath : pths) {
      setHash.bindValue(UPDATE_HASH_BY_PRIMARY_KEY_FIELD_FIRST_1024_HASH, GetFileMD5(abspath, 1024));
      setHash.bindValue(UPDATE_HASH_BY_PRIMARY_KEY_FIELD_ABSOLUTE_PATH, abspath);
      if (!setHash.exec()) {
        LOG_W("set field md5 of[`%s`] failed", qPrintable(abspath));
        continue;
      }
      ++cnt;
    }
  }
  LOG_D("Fill %d record(s) on hash fields", cnt);
  return cnt;
}

const QString DupVidsManager::READ_DUP_INFO_FROM_TABLES  //
    {QString{"SELECT `%1`, `%2`, `%3`, `%4`, `%5`, `%6`"}  //
         .arg(ENUM_2_STR(EFFECTIVE_NAME))                  //
         .arg(ENUM_2_STR(SIZE))                            //
         .arg(ENUM_2_STR(DURATION))                        //
         .arg(ENUM_2_STR(DATE))                            //
         .arg(ENUM_2_STR(ABSOLUTE_PATH))                   //
         .arg(ENUM_2_STR(FIRST_1024_HASH))                 //
     + " FROM `%1`;"};

enum READ_DUP_INFO_FROM_TABLES_FIELD {
  READ_DUP_INFO_FROM_TABLES_FIELD_EFFECTIVE_NAME = 0,
  READ_DUP_INFO_FROM_TABLES_FIELD_SIZE,
  READ_DUP_INFO_FROM_TABLES_FIELD_DURATION,
  READ_DUP_INFO_FROM_TABLES_FIELD_DATE,
  READ_DUP_INFO_FROM_TABLES_FIELD_ABSOLUTE_PATH,
  READ_DUP_INFO_FROM_TABLES_FIELD_FIRST_1024_HASH,
};

int DupVidsManager::ReadSpecifiedTables2List(const QStringList& tbls, QList<DUP_INFO>& vidsInfo) {
  if (tbls.isEmpty()) {
    vidsInfo.clear();
    return 0;
  }

  const int beforeCnt = vidsInfo.size();
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  int succeedTblCnt = 0;
  QSqlQuery query(db);
  for (const QString& tableName : tbls) {
    if (!query.exec(READ_DUP_INFO_FROM_TABLES.arg(tableName))) {
      LOG_W("qry table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      return succeedTblCnt;
    }
    while (query.next()) {
      vidsInfo.append(DUP_INFO{query.value(0).toString(),    //
                               query.value(1).toLongLong(),  //
                               query.value(2).toInt(),       //
                               query.value(3).toLongLong(),  //
                               query.value(4).toString(),    //
                               query.value(5).toString(), true});
    }
    ++succeedTblCnt;
  }
  LOG_D("before %d row(s), now %d row(s), succeed %d table(s)", beforeCnt, vidsInfo.size(), succeedTblCnt);
  return true;
}

QList<DupTableModelData> DupVidsManager::TableName2Cnt() {
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return {};
  }

  QList<DupTableModelData> tbl2Cnt;
  QSqlQuery query(db);
  for (const QString& tableName : db.tables()) {
    if (!query.exec(QString("SELECT COUNT(`EFFECTIVE_NAME`) FROM `%1`;").arg(tableName))) {
      LOG_D("count table[%s] failed[%s]", qPrintable(tableName), qPrintable(query.lastError().text()));
      continue;
    }
    int recordCnt = 0;
    while (query.next()) {
      recordCnt = query.record().value(0).toInt();
    }
    tbl2Cnt.append({tableName, recordCnt});  // query.record().count();
  }
  LOG_D("Update Tables Count:%d", tbl2Cnt.size());
  return tbl2Cnt;
}
