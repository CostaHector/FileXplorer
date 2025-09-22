#include "DbManager.h"
#include "MountHelper.h"
#include "Logger.h"
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDirIterator>
#include <QVariant>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QUrl>

const QString DbManager::DROP_TABLE_TEMPLATE{"DROP TABLE `%1`;"};
const QString DbManager::DELETE_TABLE_TEMPLATE{"DELETE FROM `%1`;"};
constexpr int DbManager::MAX_BATCH_SIZE;

QString DbManager::GetRmvCmdTemplate(DROP_OR_DELETE dropOrDelete) {
  switch (dropOrDelete) {
    case DROP_OR_DELETE::DROP:
      return DROP_TABLE_TEMPLATE;
    case DROP_OR_DELETE::DELETE:
      return DELETE_TABLE_TEMPLATE;
    default:
      return "";
  }
}

#ifdef RUNNING_UNIT_TESTS
bool DbManager::DropAllTablesForTest(const QString& connName) {
  if (!QSqlDatabase::connectionNames().contains(connName)) {
    LOG_D("connName[%s] not in pool, cannot used for table drop", qPrintable(connName));
    return true;
  }
  QSqlDatabase db = QSqlDatabase::database(connName, true);
  if (!db.isOpen() && !db.open()) {
    LOG_E("Failed to open database to drop all tables");
    return false;
  }
  db.transaction();
  const QStringList& tables = db.tables();
  LOG_D("Start to drop %d table(s)", tables.size());
  QSqlQuery query(db);

  if (!query.exec("PRAGMA foreign_keys = OFF")) {
    LOG_W("Failed to disable foreign keys: %s", qPrintable(query.lastError().text()));
  }

  for (const QString& table : tables) {
    if (!query.exec(GetRmvCmdTemplate(DROP_OR_DELETE::DROP).arg(table))) {
      LOG_W("Drop failed table[%s]: %s", qPrintable(table), qPrintable(query.lastError().text()));
      db.rollback();
      return false;
    } else {
      LOG_D("Drop ok table[%s]", qPrintable(table));
    }
  }

  if (!db.commit()) {
    LOG_E("Commit failed: %s", qPrintable(db.lastError().text()));
    db.rollback();
    return false;
  }
  return true;
}

// recommend in Singleton!
bool DbManager::DropDatabaseForTest(const QString& dbFullName, const bool bRecycle) {
  if (!dbFullName.endsWith(".db", Qt::CaseSensitivity::CaseInsensitive)) {
    LOG_E("Skip file[%s] is not .db", qPrintable(dbFullName));
    return false;
  }
  static const auto rmvIfFileExist = [](const QString& path) -> bool { return !QFile::exists(path) || QFile(path).remove(); };
  static const auto recycleIfFileExist = [](const QString& path) -> bool { return !QFile::exists(path) || QFile::moveToTrash(path); };
  static const std::function<bool(const QString&)> FileProc[2]{rmvIfFileExist, recycleIfFileExist};
  FileProc[bRecycle](dbFullName + "-shm");
  FileProc[bRecycle](dbFullName + "-wal");
  if (!FileProc[bRecycle](dbFullName)) {
    LOG_E("RecycleOrRemove .db file[%s] failed", qPrintable(dbFullName));
    return false;
  }
  return true;
}

// recommend in auto object not static/global variant the lifetime through the program
bool DbManager::DeleteDatabaseIselfForTest(bool bRecyle) {
  if (!mIsValid) {
    LOG_W("invalid cannot drop database");
    return false;
  }
  ReleaseConnection();
  DropDatabaseForTest(mDbName, bRecyle);
  mIsValid = false;
  return true;
}
#endif

DbManager::DbManager(const QString& dbName, const QString& connName, QObject* parent)  //
    : QObject{parent}, mDbName{dbName}, mConnName{connName} {                          //
  if (dbName.isEmpty() || connName.isEmpty()) {
    LOG_W("dbName[%s] or connName[%s] is empty", qPrintable(dbName), qPrintable(connName));
    return;
  }
  mIsValid = true;
}

void DbManager::ReleaseConnection() {
  if (!QSqlDatabase::contains(mConnName)) {
    LOG_D("Connection[%s] already not found in connection pool", qPrintable(mConnName));
    return;
  }
  QSqlDatabase db = QSqlDatabase::database(mConnName, false);  // by default close
  if (db.isOpen()) {
    db.close();
    LOG_D("Database connection closed: %s", qPrintable(mConnName));
  }
  // Todo:
  // QSqlDatabase::removeDatabase(mConnName); // QSqlDatabasePrivate::removeDatabase: connection 'AI_MEDIA_DUP_CONNECT' is still in use, all queries will cease to work.
}

DbManager::~DbManager() {
  if (!mIsValid) {
    return;
  }
  ReleaseConnection();  // todo
}

QSqlDatabase DbManager::GetDb(bool open) const {
  if (!mIsValid) {
    LOG_W("invalid cannot Get db");
    return {};
  }
  QSqlDatabase db;
  if (QSqlDatabase::contains(mConnName)) {
    db = QSqlDatabase::database(mConnName, open);
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", mConnName);
    db.setDatabaseName(mDbName);
  }
  if (open && !db.isOpen()) {
    if (!db.open()) {
      LOG_W("Open %s failed", qPrintable(GetCfgDebug()));
    }
  }
  return db;
}

bool DbManager::CheckValidAndOpen(QSqlDatabase& db) const {
  if (!db.isValid()) {
    LOG_W("db[%s] is invalid", qPrintable(db.connectionName()));
    return false;
  }
  if (!db.open()) {
    LOG_W("db[%s] open failed: %s",  //
          qPrintable(db.connectionName()), qPrintable(db.lastError().text()));
    return false;
  }
  return true;
}

bool DbManager::QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const {
  if (!mIsValid) {
    LOG_W("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    records << qry.record();
  }
  LOG_D("%d records find by [%s]", records.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::UpdateForTest(const QString& qryCmd) const {
  if (!mIsValid) {
    LOG_W("invalid cannot query");
    return FD_NOT_INITED;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_INVALID;
  }
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return FD_EXEC_FAILED;
  }
  int affectedRows = qry.numRowsAffected();
  LOG_D("%d records affected by [%s]", affectedRows, qPrintable(qryCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<QString>& vals) const {
  if (!mIsValid) {
    LOG_W("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toString();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<int>& vals) const {
  if (!mIsValid) {
    LOG_W("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toInt();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<qint64>& vals) const {
  if (!mIsValid) {
    LOG_W("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString qryCmd{QString{"SELECT `%1` FROM `%2`"}.arg(pk).arg(tableName)};
  QSqlQuery qry{db};
  if (!qry.exec(qryCmd)) {
    LOG_W("cmd[%s] failed:%s", qPrintable(qry.lastQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toLongLong();
  }
  LOG_D("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::CountRow(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return FD_INVALID;
  }

  QString countCmd = QString("SELECT COUNT(*) FROM `%1`").arg(tableName);
  if (!whereClause.isEmpty()) {
    countCmd += (" WHERE " + whereClause);
  }

  QSqlQuery qry{db};
  if (!qry.exec(countCmd)) {
    LOG_W("count[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  qry.next();
  return qry.value(0).toInt();
}

bool DbManager::IsTableEmpty(const QString& tableName) const {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return true;
  }
  QSqlQuery qry{db};
  qry.prepare(QString{"SELECT * FROM `%1`"}.arg(tableName));
  if (!qry.exec()) {
    LOG_W("select[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
  }
  return !qry.next();
}

int DbManager::DeleteByWhereClause(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    LOG_W("Open failed:%s", qPrintable(db.lastError().text()));
    return FD_DB_OPEN_FAILED;
  }

  QString deleteCmd{QString(R"(DELETE FROM "%1")").arg(tableName)};
  if (!whereClause.isEmpty()) {
    deleteCmd += " WHERE ";
    deleteCmd += whereClause;
  }
  deleteCmd += ';';

  QSqlQuery qry{db};
  if (!qry.exec(deleteCmd)) {
    db.rollback();
    LOG_W("delete cmd[%s] failed: %s",  //
          qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  const int affectedRows = qry.numRowsAffected();
  LOG_D("affectedRows:%d by cmd:%s", affectedRows, qPrintable(deleteCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::IsTableVolumeOnline(const QString& tableName) const {
  QString stdGuidPart{tableName};
  stdGuidPart.replace(MountHelper::TABLE_UNDERSCORE, MountHelper::GUID_HYPEN);
  return MountHelper::isVolumeAvailable(stdGuidPart);
}

bool DbManager::onShowInFileSystemView() const {
  if (!QFile::exists(mDbName)) {
    LOG_W("Database[%s] not exist, open failed", qPrintable(mDbName));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(mDbName));
}

QString DbManager::GetDeleteInPlaceholders(int n) {
  QString placeholders;
  placeholders.reserve(n * 2);
  for (int i = 0; i < n; ++i) {
    placeholders += "?,";
  }
  if (!placeholders.isEmpty()) {  // chop last 1 char(s) ","
    static constexpr int LAST_CHARS_CNT = 1;
    placeholders.chop(LAST_CHARS_CNT);
  }
  return placeholders;
}

// not full match
bool DbManager::IsMatch(const QString& s, const QRegularExpression& regex) {
  return regex.isValid() && regex.match(s).hasMatch();
}

bool DbManager::CreateDatabase() {
  if (!mIsValid) {
    LOG_W("invalid cannot create database");
    return false;
  }
  const auto db = GetDb();
  return db.isValid();
}

bool DbManager::CreateTable(const QString& tableName, const QString& tableDefinitionTemplate) {
  if (!mIsValid) {
    LOG_W("invalid cannot create table[%s]", qPrintable(tableName));
    return false;
  }

  if (!tableDefinitionTemplate.contains("%1")) {
    LOG_W("table definition[%s] invalid", qPrintable(tableDefinitionTemplate));
    return false;
  }
  const QString& tableDefinition = tableDefinitionTemplate.arg(tableName);
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }

  if (db.tables().contains(tableName)) {
    return true;
  }

  // 启用外键支持和WAL模式提升性能
  QSqlQuery query(db);
  query.exec("PRAGMA journal_mode = WAL");  // WAL/DELETE, the former may generate .db-wal file
  query.exec("PRAGMA foreign_keys = ON");
  query.exec("PRAGMA synchronous = NORMAL");
  if (!query.exec(tableDefinition)) {
    LOG_W("Create table[%s] failed: %s", qPrintable(tableDefinition), qPrintable(query.lastError().text()));
    return false;  // db and query will destroyed when out of scope
  }
  LOG_D("Table[%s] create succeed", qPrintable(tableName));
  return true;
}

int DbManager::RmvTable(const QString& tableNameRegexPattern, DROP_OR_DELETE dropOrDelete, bool isFullMatch) {
  if (!mIsValid) {
    LOG_W("invalid cannot drop table");
    return FD_NOT_INITED;
  }
  QString matchStr{tableNameRegexPattern};
  if (isFullMatch) {
    matchStr.push_front('^');
    matchStr.push_back('$');
  }
  const QRegularExpression regex{matchStr};
  if (!regex.isValid()) {
    LOG_W("regex[%s] is invalid", qPrintable(tableNameRegexPattern));
    return FD_TABLE_NAME_PATTERN_INVALID;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  const QString rmvCmdTemplate{GetRmvCmdTemplate(dropOrDelete)};
  if (rmvCmdTemplate.isEmpty()) {
    LOG_W("rmvCmdTemplate empty. mode[%d] invalid", (int)dropOrDelete);
    return FD_DB_INVALID;
  }

  int succeedDropCnt = 0;
  QSqlQuery query(db);
  const QStringList& allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (!IsMatch(tableName, regex)) {
      continue;
    }
    if (!query.exec(rmvCmdTemplate.arg(tableName))) {
      LOG_W("Drop table[%s] failed: %s",  //
            qPrintable(tableName), qPrintable(query.lastError().text()));
      db.rollback();
      return FD_EXEC_FAILED;
    }
    ++succeedDropCnt;
  }
  LOG_D("Drop %d table(s) from %d table(s) by user specified pattern[%s]",  //
        succeedDropCnt, allTables.size(), qPrintable(tableNameRegexPattern));
  return succeedDropCnt;
}
