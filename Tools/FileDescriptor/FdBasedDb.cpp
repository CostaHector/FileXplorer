#include "FdBasedDb.h"
#include "FileDescriptor.h"
#include "JsonHelper.h"
#include "VideoDurationGetter.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>

using namespace MOVIE_TABLE;

QStringList FdBasedDb::VIDEOS_FILTER = TYPE_FILTER::VIDEO_TYPE_SET;
bool FdBasedDb::mSkipGetDuration{false};
bool FdBasedDb::CHECK_TABLE_VOLUME_ONLINE{true};

const QString FdBasedDb::CREATE_TABLE_TEMPLATE  //
    {
        "CREATE TABLE IF NOT EXISTS `%1` ("          // TABLE_NAME
        + QString{"`%1` BIGINT NOT NULL, "           // fd BIGINT
                  "`%2` NCHAR(260) DEFAULT '', "     // PrePathLeft
                  "`%3` NCHAR(260) DEFAULT '', "     // PrePathRight
                  "`%4` NCHAR(260) NOT NULL, "       // Name
                  "`%5` INTEGER DEFAULT 0, "         // Size
                  "`%6` INTEGER DEFAULT 0, "         // Duration
                  "`%7` VARCHAR(100) DEFAULT '', "   // Studio
                  "`%8` VARCHAR(260) DEFAULT '', "   // Cast must seperated by comma only
                  "`%9` VARCHAR(260) DEFAULT '', "   // Tags must seperated by comma only
                  "`%10` INTEGER UNIQUE NOT NULL, "  // PathHash
                  "PRIMARY KEY (%1, %3, %4)"
                  ");"}
              .arg(ENUM_2_STR(Fd))            //
              .arg(ENUM_2_STR(PrePathLeft))   //
              .arg(ENUM_2_STR(PrePathRight))  //
              .arg(ENUM_2_STR(Name))          //
              .arg(ENUM_2_STR(Size))          //
              .arg(ENUM_2_STR(Duration))      //
              .arg(ENUM_2_STR(Studio))        //
              .arg(ENUM_2_STR(Cast))          //
              .arg(ENUM_2_STR(Tags))          //
              .arg(ENUM_2_STR(PathHash))      //
    };

const QString FdBasedDb::INSERT_MOVIE_RECORD_FULL_TEMPLATE  //
    {
        "REPLACE INTO `%1` "                                                        // TABLE_NAME
        + QString{"(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`, `%8`, `%9`, `%10`) "  //
                  "VALUES "                                                         //
                  "(:%1, :%2, :%3, :%4, :%5, :%6, :%7, :%8, :%9, :%10);"}           //
              .arg(ENUM_2_STR(Fd))                                                  //
              .arg(ENUM_2_STR(PrePathLeft))                                         //
              .arg(ENUM_2_STR(PrePathRight))                                        //
              .arg(ENUM_2_STR(Name))                                                //
              .arg(ENUM_2_STR(Size))                                                //
              .arg(ENUM_2_STR(Duration))                                            //
              .arg(ENUM_2_STR(Studio))                                              //
              .arg(ENUM_2_STR(Cast))                                                //
              .arg(ENUM_2_STR(Tags))                                                //
              .arg(ENUM_2_STR(PathHash))                                            //
    };

const QString FdBasedDb::INSERT_MOVIE_RECORD_TEMPLATE  //
    {
        "REPLACE INTO `%1` "                               // TABLE_NAME
        + QString{"(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`) "  //
                  "VALUES "                                //
                  "(:%1, :%2, :%3, :%4, :%5, :%6);"}       //
              .arg(ENUM_2_STR(Fd))                         //
              .arg(ENUM_2_STR(PrePathLeft))                //
              .arg(ENUM_2_STR(PrePathRight))               //
              .arg(ENUM_2_STR(Name))                       //
              .arg(ENUM_2_STR(Size))                       //
              .arg(ENUM_2_STR(PathHash))                   //
    };

enum INSERT_FIELD {
  INSERT_FIELD_Fd = 0,        //
  INSERT_FIELD_PrePathLeft,   //
  INSERT_FIELD_PrePathRight,  //
  INSERT_FIELD_Name,          //
  INSERT_FIELD_Size,          //
  INSERT_FIELD_PathHash,      //
};

enum UPDATE_PATH_FIELED {
  UPDATE_PATH_FILED_PrePathLeft = 0,  //
  UPDATE_PATH_FILED_PrePathRight,     //
  UPDATE_PATH_FILED_Name,             //
  UPDATE_PATH_FILED_PathHash,         //
  UPDATE_PATH_FILED_Fd
};

const QString FdBasedDb::UPDATE_PATH_TEMPLATE  //
    {
        "UPDATE `%1` "  //
        + QString("SET `%1` = :%1, `%2` = :%2, `%3` = :%3, `%4` = :%4 "
                  "WHERE `%5` = :%5;")
              .arg(ENUM_2_STR(PrePathLeft))   //
              .arg(ENUM_2_STR(PrePathRight))  //
              .arg(ENUM_2_STR(Name))          //
              .arg(ENUM_2_STR(PathHash))      //
              .arg(ENUM_2_STR(Fd))            //
    };

enum QUERY_DURATION_0_FILED {
  QUERY_DURATION_0_FILED_PrePathLeft = 0,  //
  QUERY_DURATION_0_FILED_PrePathRight,     //
  QUERY_DURATION_0_FILED_Name,             //
  QUERY_DURATION_0_FILED_Fd
};

const QString FdBasedDb::SELECT_DURATION_0_TEMPLATE  //
    {
        QString{"SELECT `%1`, `%2`, `%3`, `%4` FROM"}  //
            .arg(ENUM_2_STR(PrePathLeft))              //
            .arg(ENUM_2_STR(PrePathRight))             //
            .arg(ENUM_2_STR(Name))                     //
            .arg(ENUM_2_STR(Fd))                       //
        + " `%1` "                                     //
        + QString{"WHERE `%1` = 0;"}                   //
              .arg(ENUM_2_STR(Duration))               //
    };

enum UPDATE_DURATION_0_FILED {
  UPDATE_DURATION_0_FILED_Duration = 0,  //
  UPDATE_DURATION_0_FILED_Fd
};

const QString FdBasedDb::UPDATE_DURATION_0_TEMPLATE  //
    {
        "UPDATE `%1` "  //
        + QString("SET `%1` = :%1 "
                  "WHERE `%2` = :%2;")
              .arg(ENUM_2_STR(Duration))  //
              .arg(ENUM_2_STR(Fd))        //
    };

const QString FdBasedDb::SELECT_DURATION_STUDIO_CAST_TAGS_TEMPLATE  //
    {
        QString{"SELECT `%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7` FROM"}             //
            .arg(ENUM_2_STR(PrePathLeft))                                           //
            .arg(ENUM_2_STR(PrePathRight))                                          //
            .arg(ENUM_2_STR(Name))                                                  //
            .arg(ENUM_2_STR(Duration))                                              //
            .arg(ENUM_2_STR(Studio))                                                //
            .arg(ENUM_2_STR(Cast))                                                  //
            .arg(ENUM_2_STR(Tags))                                                  //
        + " `%1` "                                                                  //
        + QString{R"(WHERE `%1` != 0 OR `%2` != '' OR `%3` != '' OR `%4` != '';)"}  //
              .arg(ENUM_2_STR(Duration))                                            //
              .arg(ENUM_2_STR(Studio))                                              //
              .arg(ENUM_2_STR(Cast))                                                //
              .arg(ENUM_2_STR(Tags))                                                //
    };

enum UPDATE_STUDIO_CAST_TAGS_FIELED {
  UPDATE_STUDIO_CAST_TAGS_Studio = 0,  //
  UPDATE_STUDIO_CAST_TAGS_Cast,        //
  UPDATE_STUDIO_CAST_TAGS_Tags,        //
  UPDATE_STUDIO_CAST_TAGS_PathHash,    //
};

const QString FdBasedDb::UPDATE_STUDIO_CAST_TAGS_TEMPLATE  //
    {
        "UPDATE `%1` "                                       //
        + QString("SET `%1` = :%1, `%2` = :%2, `%3` = :%3 "  //
                  "WHERE `%4` = :%4;")                       //
              .arg(ENUM_2_STR(Studio))                       //
              .arg(ENUM_2_STR(Cast))                         //
              .arg(ENUM_2_STR(Tags))                         //
              .arg(ENUM_2_STR(PathHash))                     //
    };

enum EXPORT_TO_JSON {
  EXPORT_TO_JSON_FIELD_PrePathLeft = 0,  //
  EXPORT_TO_JSON_FIELD_PrePathRight,     //
  EXPORT_TO_JSON_FIELD_Name,             //
  EXPORT_TO_JSON_FIELD_Duration,         //
  EXPORT_TO_JSON_FIELD_Studio,           //
  EXPORT_TO_JSON_FIELD_Cast,             //
  EXPORT_TO_JSON_FIELD_Tags,             //
};

const QString FdBasedDb::QUERY_KEY_INFO_TEMPLATE  //
    {
        QString{"SELECT `%1`, `%2`, `%3`, `%4` FROM"}  //
            .arg(ENUM_2_STR(PrePathLeft))              //
            .arg(ENUM_2_STR(PrePathRight))             //
            .arg(ENUM_2_STR(Name))                     //
            .arg(ENUM_2_STR(Size))                     //
        + " `%1` WHERE %2"                             //
    };

const QString FdBasedDb::WHERE_NAME_CORE_TEMPLATE{//
                                                  QString{R"(`%1` LIKE )"}.arg(ENUM_2_STR(Name)) + R"("%1")"};

// Incremental
int FdBasedDb::ReadADirectory(const QString& tableName, const QString& folderAbsPath) {
  if (tableName.isEmpty()) {
    return FD_TABLE_NAME_INVALID;
  }

  if (!QFileInfo(folderAbsPath).isDir()) {
    LOG_W("folderAbsPath[%s] is not a directory", qPrintable(folderAbsPath));
    return FD_NOT_DIR;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  if (!db.tables().contains(tableName)) {
    return FD_TABLE_INEXIST;
  }

  // 1. query fd(s) from table
  QSet<qint64> existedFds;
  if (!QueryPK(tableName, ENUM_2_STR(Fd), existedFds)) {
    LOG_W("Qry fds(s) at table[%s] failed", qPrintable(tableName));
    return FD_QRY_PK_FAILED;
  }

  // 2. fd->absolute file path
  FileDescriptor fd;
  QHash<qint64, QString> newFd2Pth;
  qint64 fdVal{0};
  QDirIterator it{folderAbsPath, VIDEOS_FILTER, QDir::Files, QDirIterator::Subdirectories};
  QString absFilePath;
  while (it.hasNext()) {
    it.next();
    absFilePath = it.filePath();
    fdVal = fd.GetFileUniquedId(absFilePath);
    if (fdVal <= 0) {
      continue;
    }
    newFd2Pth[fdVal] = absFilePath;
  }
  const QList<qint64>& newFdLst = newFd2Pth.keys();
  const QSet<qint64> newFds{newFdLst.cbegin(), newFdLst.cend()};

  auto needInsertFds{newFds};
  needInsertFds.subtract(existedFds);

  int insertCnt = 0;
  auto ret = Insert(tableName, needInsertFds, newFd2Pth, insertCnt);
  if (ret != FD_OK) {
    LOG_W("Incremental insert failed errorCode:%d", ret);
    return ret;
  }

  LOG_O("%d record(s) commit insert into succeed", insertCnt);
  return insertCnt;
}

FD_ERROR_CODE FdBasedDb::Insert(const QString& tableName,                 //
                                const QSet<qint64>& needInsertFds,        //
                                const QHash<qint64, QString>& newFd2Pth,  //
                                int& insertCnt) {
  auto db = GetDb();
  insertCnt = 0;
  if (needInsertFds.isEmpty()) {
    return FD_OK;
  }
  QSqlQuery query{db};
  if (!query.prepare(INSERT_MOVIE_RECORD_TEMPLATE.arg(tableName))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s",  //
          1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  int count = 0;
  QString absFilePath;
  for (const qint64 fdVal : needInsertFds) {
    absFilePath = newFd2Pth[fdVal];
    const PathTool::RMFComponent& rmf = PathTool::RMFComponent::FromPath(absFilePath);
    // 绑定参数
    query.bindValue(INSERT_FIELD_Fd, fdVal);
    query.bindValue(INSERT_FIELD_PrePathLeft, rmf.rootPart);
    query.bindValue(INSERT_FIELD_PrePathRight, rmf.middlePart);
    query.bindValue(INSERT_FIELD_Name, rmf.fileName);
    query.bindValue(INSERT_FIELD_Size, QFile{absFilePath}.size());
    query.bindValue(INSERT_FIELD_PathHash, JsonHelper::CalcFileHash(absFilePath));

    if (!query.exec()) {
      db.rollback();
      LOG_W("replace[%s] failed: %s",  //
            qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_EXEC_FAILED;
    }

    count++;
    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        LOG_W("commit the %dth batch record(s) failed: %s",  //
              count / MAX_BATCH_SIZE + 1, qPrintable(db.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        LOG_W("start the %dth transaction failed: %s",  //
              count / MAX_BATCH_SIZE + 2, qPrintable(db.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    LOG_W("remain record(s) commit failed: %s", qPrintable(db.lastError().text()));
    return FD_COMMIT_FAILED;
  }
  query.finish();
  insertCnt = needInsertFds.size();
  LOG_D("%d record(s) to be inserted...", insertCnt);
  return FD_OK;
}

FD_ERROR_CODE FdBasedDb::Delete(const QString& tableName, const QSet<qint64>& needDeleteFds, int& deleteCnt) {
  auto db = GetDb();
  deleteCnt = 0;
  if (needDeleteFds.isEmpty()) {
    return FD_OK;
  }
  const QString& placeholders = GetDeleteInPlaceholders(needDeleteFds.size());
  const QString qryCmd = QString{R"(DELETE FROM `%1` WHERE `%2` IN (%3);)"}
                             .arg(tableName)       //
                             .arg(ENUM_2_STR(Fd))  //
                             .arg(placeholders);
  QSqlQuery query{db};
  if (!query.prepare(qryCmd)) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  for (const qint64& fdVal : needDeleteFds) {
    query.addBindValue(fdVal);
  }

  if (!query.exec()) {
    db.rollback();
    LOG_W("delete[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_INVALID;
  }
  query.finish();
  deleteCnt = needDeleteFds.size();
  LOG_D("%d record(s) to be deleted...", deleteCnt);
  return FD_OK;
}

FD_ERROR_CODE FdBasedDb::Update(const QString& tableName,
                                const QSet<qint64>& needUpdateFds,
                                const QHash<qint64, QString>& newFd2Pth,
                                int& updateCnt) {
  auto db = GetDb();
  updateCnt = 0;
  if (needUpdateFds.isEmpty()) {
    return FD_OK;
  }
  QSqlQuery query{db};
  if (!query.prepare(UPDATE_PATH_TEMPLATE.arg(tableName))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  // 开始事务
  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s",  //
          1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }
  int count = 0;
  QString absFilePath;
  for (const qint64& fdVal : needUpdateFds) {
    absFilePath = newFd2Pth[fdVal];
    const PathTool::RMFComponent& rmf = PathTool::RMFComponent::FromPath(absFilePath);
    query.bindValue(UPDATE_PATH_FILED_PrePathLeft, rmf.rootPart);
    query.bindValue(UPDATE_PATH_FILED_PrePathRight, rmf.middlePart);
    query.bindValue(UPDATE_PATH_FILED_Name, rmf.fileName);
    query.bindValue(UPDATE_PATH_FILED_PathHash, JsonHelper::CalcFileHash(absFilePath));
    query.bindValue(UPDATE_PATH_FILED_Fd, fdVal);
    if (!query.exec()) {
      db.rollback();
      LOG_W("update[%s] failed: %s",  //
            qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_INVALID;
    }

    count++;
    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        LOG_W("commit the %dth batch record(s) failed: %s",  //
              count / MAX_BATCH_SIZE + 1, qPrintable(db.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        LOG_W("start the %dth transaction failed: %s",  //
              count / MAX_BATCH_SIZE + 2, qPrintable(db.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    LOG_W("remain update record(s) commit failed: %s", qPrintable(db.lastError().text()));
    return FD_COMMIT_FAILED;
  }
  query.finish();
  updateCnt = needUpdateFds.size();
  LOG_D("%d record(s) to be update...", updateCnt);
  return FD_OK;
}

// PeerPathTable
FD_ERROR_CODE FdBasedDb::Adt(const QString& tableName, const QString& peerPath, VolumeUpdateResult* pAdt) {
  if (tableName.isEmpty()) {
    return FD_TABLE_NAME_INVALID;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  if (!db.tables().contains(tableName)) {
    return FD_TABLE_INEXIST;
  }

  // 1. query fd(s) from table
  QSet<qint64> existedFds;
  if (!QueryPK(tableName, ENUM_2_STR(Fd), existedFds)) {
    LOG_W("Qry fds(s) at table[%s] failed", qPrintable(tableName));
    return FD_QRY_PK_FAILED;
  }

  // 2. fd->absolute file path
  FileDescriptor fd;
  QHash<qint64, QString> newFd2Pth;
  qint64 fdVal{0};
  QDirIterator it{peerPath, VIDEOS_FILTER, QDir::Files, QDirIterator::Subdirectories};
  QString absFilePath;
  while (it.hasNext()) {
    it.next();
    absFilePath = it.filePath();
    fdVal = fd.GetFileUniquedId(absFilePath);
    if (fdVal <= 0) {
      continue;
    }
    newFd2Pth[fdVal] = absFilePath;
  }
  const QList<qint64>& newFdLst = newFd2Pth.keys();
  const QSet<qint64> newFds{newFdLst.cbegin(), newFdLst.cend()};

  auto needInsertFds{newFds};
  needInsertFds.subtract(existedFds);
  auto needDeleteFds{existedFds};
  needDeleteFds.subtract(newFds);
  auto needUpdateFds{newFds};
  needUpdateFds.intersect(existedFds);
  LOG_D("Fds insert:%d, delete:%d, update:%d", needInsertFds.size(), needDeleteFds.size(), needUpdateFds.size());

  // 3. before insert check if at least 1 guid
  int insertCnt{0};
  auto ret = Insert(tableName, needInsertFds, newFd2Pth, insertCnt);
  if (ret != FD_OK) {
    LOG_W("Insert failed errorCode:%d", ret);
    return ret;
  }
  // 4. before delete check if at least 1 guid
  int deleteCnt{0};
  ret = Delete(tableName, needDeleteFds, deleteCnt);
  if (ret != FD_OK) {
    LOG_W("Delete failed errorCode:%d", ret);
    return ret;
  }
  // 5. before update check if at least 1 guid
  int updateCnt{0};
  ret = Update(tableName, needUpdateFds, newFd2Pth, updateCnt);
  if (ret != FD_OK) {
    LOG_W("Delete failed errorCode:%d", ret);
    return ret;
  }

  if (pAdt != nullptr) {
    pAdt->insertCnt = insertCnt;
    pAdt->deleteCnt = deleteCnt;
    pAdt->updateCnt = updateCnt;
  }

  LOG_D("insert:%d, delete:%d, update:%d record(s) succeed", insertCnt, deleteCnt, updateCnt);
  return FD_OK;
}

int FdBasedDb::SetDuration(const QString& tableName) {
  if (mSkipGetDuration) {
    return FD_OK;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  if (!db.tables().contains(tableName)) {
    return FD_OK;  // no need set duration
  }

  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    return FD_INVALID;
  }

  // 1. start to query
  QSqlQuery query{db};
  query.setForwardOnly(true);
  if (!query.exec(SELECT_DURATION_0_TEMPLATE.arg(tableName))) {
    LOG_W("Query[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_EXEC_FAILED;
  }

  QMap<qint64, int> fd2Duration;
  QString absFilePath;
  qint64 fdVal{0};
  while (query.next()) {
    absFilePath = PathTool::RMFComponent::join(query.value(QUERY_DURATION_0_FILED_PrePathLeft).toString(),   //
                                               query.value(QUERY_DURATION_0_FILED_PrePathRight).toString(),  //
                                               query.value(QUERY_DURATION_0_FILED_Name).toString());
    fdVal = query.value(QUERY_DURATION_0_FILED_Fd).toLongLong();
    fd2Duration[fdVal] = mSkipGetDuration ? 0 : mi.GetLengthQuick(absFilePath);
  }
  query.clear();
  if (fd2Duration.isEmpty()) {
    LOG_D("no duration need update at all, skip");
    return 0;
  }

  // 2. start to update
  if (!query.prepare(UPDATE_DURATION_0_TEMPLATE.arg(tableName))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s",  //
          1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  int count = 0;
  for (auto it = fd2Duration.cbegin(); it != fd2Duration.cend(); ++it) {
    query.bindValue(UPDATE_DURATION_0_FILED_Duration, it.value());
    query.bindValue(UPDATE_DURATION_0_FILED_Fd, it.key());
    if (!query.exec()) {
      db.rollback();
      LOG_W("replace[%s] failed: %s",  //
            qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_EXEC_FAILED;
    }

    count++;
    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        LOG_W("commit the %dth batch record(s) failed: %s",  //
              count / MAX_BATCH_SIZE + 1, qPrintable(db.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        LOG_W("start the %dth transaction failed: %s",  //
              count / MAX_BATCH_SIZE + 2, qPrintable(db.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    LOG_W("remain record(s) commit failed: %s", qPrintable(db.lastError().text()));
    return FD_COMMIT_FAILED;
  }
  query.finish();
  LOG_D("%d record(s) to be updated", fd2Duration.size());
  return fd2Duration.size();
}

struct DurStudioCastTags {
  int Duration;
  QString Studio;
  QString Cast;
  QString Tags;
};

int FdBasedDb::ExportDurationStudioCastTagsToJson(const QString& tableName) const {
  if (CHECK_TABLE_VOLUME_ONLINE) {
    if (!IsTableVolumeOnline(tableName)) {
      return FD_DISK_OFFLINE;
    }
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  if (!db.tables().contains(tableName)) {
    return FD_OK;  // no need set duration
  }

  QSqlQuery query{db};
  if (!query.prepare(SELECT_DURATION_STUDIO_CAST_TAGS_TEMPLATE.arg(tableName))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  if (!query.exec()) {
    LOG_W("Query[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_EXEC_FAILED;
  }

  QMap<QString, DurStudioCastTags> pth2Info;
  QString jsonAbsFilePath;
  QString jsonFileName;
  while (query.next()) {
    jsonFileName = PathTool::FileExtReplacedWithJson(query.value(EXPORT_TO_JSON_FIELD_Name).toString());       //
    jsonAbsFilePath = PathTool::RMFComponent::join(query.value(EXPORT_TO_JSON_FIELD_PrePathLeft).toString(),   //
                                                   query.value(EXPORT_TO_JSON_FIELD_PrePathRight).toString(),  //
                                                   jsonFileName);
    pth2Info[jsonAbsFilePath] = DurStudioCastTags{query.value(EXPORT_TO_JSON_FIELD_Duration).toInt(),   //
                                                  query.value(EXPORT_TO_JSON_FIELD_Studio).toString(),  //
                                                  query.value(EXPORT_TO_JSON_FIELD_Cast).toString(),    //
                                                  query.value(EXPORT_TO_JSON_FIELD_Tags).toString()};
  }
  query.clear();
  if (pth2Info.isEmpty()) {
    LOG_D("No need export to json, skip");
    return FD_OK;
  }

  int jsonFilesCnt = 0;
  using namespace JsonHelper;
  for (auto it = pth2Info.cbegin(); it != pth2Info.cend(); ++it) {
    const auto& info = it.value();
    // if json file not exist, will create it first
    const RET_ENUM& ansRet = InsertOrUpdateDurationStudioCastTags(it.key(),       //
                                                                  info.Duration,  //
                                                                  info.Studio,    //
                                                                  info.Cast,      //
                                                                  info.Tags);
    if (ansRet == CHANGED_OK) {
      ++jsonFilesCnt;
    }
  }

  LOG_D("%d json(s) file updated succeed", jsonFilesCnt);
  return jsonFilesCnt;
}

int FdBasedDb::UpdateStudioCastTagsByJson(const QString& tableName, const QString& peerPath) const {
  if (CHECK_TABLE_VOLUME_ONLINE) {
    if (!IsTableVolumeOnline(tableName)) {
      LOG_W("Table Volum offline");
      return FD_DISK_OFFLINE;
    }
  }
  using namespace JsonHelper;
  const QMap<uint, JsonDict2Table>& fileNameHash2Dict = ReadStudioCastTagsOut(peerPath);
  if (fileNameHash2Dict.isEmpty()) {
    return FD_OK;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  if (!db.tables().contains(tableName)) {
    return FD_OK;  // no need update studio/cast/tags
  }

  QSqlQuery query{db};
  if (!query.prepare(UPDATE_STUDIO_CAST_TAGS_TEMPLATE.arg(tableName))) {
    LOG_W("prepare command[%s] failed: %s",  //
          qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  // 开始事务
  if (!db.transaction()) {
    LOG_W("start the %dth transaction failed: %s",  //
          1, qPrintable(db.lastError().text()));
    return FD_TRANSACTION_FAILED;
  }

  int count{0};
  for (auto it = fileNameHash2Dict.cbegin(); it != fileNameHash2Dict.cend(); ++it) {
    const auto& dictInfo = it.value();
    query.bindValue(UPDATE_STUDIO_CAST_TAGS_Studio, dictInfo.Studio);
    query.bindValue(UPDATE_STUDIO_CAST_TAGS_Cast, dictInfo.Cast.join(ELEMENT_JOINER));
    query.bindValue(UPDATE_STUDIO_CAST_TAGS_Tags, dictInfo.Tags.join(ELEMENT_JOINER));
    query.bindValue(UPDATE_STUDIO_CAST_TAGS_PathHash, it.key());
    if (!query.exec()) {
      db.rollback();
      LOG_W("update[%s] failed: %s",  //
            qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_INVALID;
    }

    count++;
    // 分批提交
    if (count % MAX_BATCH_SIZE == 0) {
      if (!db.commit()) {
        db.rollback();
        LOG_W("commit the %dth batch record(s) failed: %s",  //
              count / MAX_BATCH_SIZE + 1, qPrintable(db.lastError().text()));
        return FD_COMMIT_FAILED;
      }
      if (!db.transaction()) {
        LOG_W("start the %dth transaction failed: %s",  //
              count / MAX_BATCH_SIZE + 2, qPrintable(db.lastError().text()));
        return FD_TRANSACTION_FAILED;
      }
    }
  }

  // 提交剩余记录
  if (!db.commit()) {
    db.rollback();
    LOG_W("remain update record(s) commit failed: %s", qPrintable(db.lastError().text()));
    return FD_COMMIT_FAILED;
  }
  query.finish();

  LOG_D("%d record(s) updated succeed", fileNameHash2Dict.size());
  return fileNameHash2Dict.size();
}
