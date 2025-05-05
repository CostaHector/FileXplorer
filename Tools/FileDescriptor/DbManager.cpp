#include "DbManager.h"
#include "Tools/FileDescriptor/MountHelper.h"
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDirIterator>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>

const QString DbManager::DROP_TABLE_TEMPLATE{"DROP TABLE `%1`;"};
const QString DbManager::DELETE_TABLE_TEMPLATE{"DELETE FROM `%1`;"};
constexpr int DbManager::MAX_BATCH_SIZE;

DbManager::DbManager(const QString& dbName, const QString& connName, QObject* parent)  //
    : QObject{parent}, mDbName{dbName}, mConnName{connName} {                          //
  if (dbName.isEmpty() || connName.isEmpty()) {
    qWarning("dbName[%s] or connName[%s] is empty", qPrintable(dbName), qPrintable(connName));
    return;
  }
  mIsValid = true;
}

void DbManager::ReleaseConnection() {
  // will not check. user must check if mConnName is invalid
  if (QSqlDatabase::contains(mConnName)) {
    auto db = QSqlDatabase::database(mConnName);
    if (db.isOpen()) {
      db.close();
    }
    db = QSqlDatabase{};
    //  QSqlDatabase::removeDatabase(mConnName);
  }
}

DbManager::~DbManager() {
  if (!mIsValid) {
    return;
  }
  ReleaseConnection();
}

QSqlDatabase DbManager::GetDb(bool open) const {
  if (!mIsValid) {
    qWarning("invalid cannot Get db");
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
      qWarning("Open %s failed", qPrintable(GetCfgDebug()));
    }
  }
  return db;
}

bool DbManager::CheckValidAndOpen(QSqlDatabase& db) const {
  if (!db.isValid()) {
    qWarning("db[%s] is invalid", qPrintable(db.connectionName()));
    return false;
  }
  if (!db.open()) {
    qWarning("db[%s] open failed: %s",  //
             qPrintable(db.connectionName()), qPrintable(db.lastError().text()));
    return false;
  }
  return true;
}

bool DbManager::QueryForTest(const QString& qryCmd, QList<QSqlRecord>& records) const {
  if (!mIsValid) {
    qWarning("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    qWarning("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    records << qry.record();
  }
  qDebug("%d records find by [%s]", records.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::UpdateForTest(const QString& qryCmd) const {
  if (!mIsValid) {
    qWarning("invalid cannot query");
    return FD_NOT_INITED;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_INVALID;
  }
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    qWarning("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return FD_EXEC_FAILED;
  }
  int affectedRows = qry.numRowsAffected();
  qDebug("%d records affected by [%s]", affectedRows, qPrintable(qryCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<QString>& vals) const {
  if (!mIsValid) {
    qWarning("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString& qryCmd = QString{"SELECT `%1` FROM %2"}.arg(pk).arg(tableName);
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    qWarning("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toString();
  }
  qDebug("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<int>& vals) const {
  if (!mIsValid) {
    qWarning("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString& qryCmd = QString{"SELECT `%1` FROM %2"}.arg(pk).arg(tableName);
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    qWarning("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toInt();
  }
  qDebug("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

bool DbManager::QueryPK(const QString& tableName, const QString& pk, QSet<qint64>& vals) const {
  if (!mIsValid) {
    qWarning("invalid cannot query");
    return false;
  }
  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }
  const QString& qryCmd = QString{"SELECT `%1` FROM %2"}.arg(pk).arg(tableName);
  QSqlQuery qry{qryCmd, db};
  if (!qry.exec()) {
    qWarning("cmd[%s] failed:%s", qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    db.rollback();
    return false;
  }
  while (qry.next()) {
    vals << qry.value(pk).toLongLong();
  }
  qDebug("%d records find by [%s]", vals.size(), qPrintable(qryCmd));
  return true;
}

int DbManager::CountRow(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    qWarning("Open failed:%s", qPrintable(db.lastError().text()));
    return FD_INVALID;
  }

  QString countCmd = QString("SELECT COUNT(*) FROM %1").arg(tableName);
  if (!whereClause.isEmpty()) {
    countCmd += (" WHERE " + whereClause);
  }

  QSqlQuery qry{db};
  if (!qry.exec(countCmd)) {
    qWarning("count[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  qry.next();
  return qry.value(0).toInt();
}

bool DbManager::IsTableEmpty(const QString& tableName) const {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    qWarning("Open failed:%s", qPrintable(db.lastError().text()));
    return true;
  }
  QSqlQuery qry{db};
  qry.prepare(QString{"SELECT * FROM %1"}.arg(tableName));
  if (!qry.exec()) {
    qWarning("select[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
  }
  return !qry.next();
}

int DbManager::DeleteByWhereClause(const QString& tableName, const QString& whereClause) {
  QSqlDatabase db = GetDb();
  if (!CheckValidAndOpen(db)) {
    qWarning("Open failed:%s", qPrintable(db.lastError().text()));
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
    qWarning("delete cmd[%s] failed: %s",  //
             qPrintable(qry.executedQuery()), qPrintable(qry.lastError().text()));
    return FD_EXEC_FAILED;
  }
  const int affectedRows = qry.numRowsAffected();
  qDebug("affectedRows:%d by cmd:%s", affectedRows, qPrintable(deleteCmd));
  qry.finish();
  return affectedRows;
}

bool DbManager::IsTableVolumeOnline(const QString& tableName) const {
  QString stdGuidPart{tableName};
  stdGuidPart.replace(MountHelper::TABLE_UNDERSCORE, MountHelper::GUID_HYPEN);
  return MountHelper::isVolumeAvailable(stdGuidPart);
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
    qWarning("invalid cannot create database");
    return false;
  }
  const auto db = GetDb();
  return db.isValid();
}

bool DbManager::CreateTable(const QString& tableName, const QString& tableDefinitionTemplate) {
  if (!mIsValid) {
    qWarning("invalid cannot create table[%s]", qPrintable(tableName));
    return false;
  }

  if (!tableDefinitionTemplate.contains("%1")) {
    qWarning("tableDefinitionTemplate [%s] invalid", qPrintable(tableDefinitionTemplate));
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
  query.exec("PRAGMA foreign_keys = ON");
  query.exec("PRAGMA journal_mode = WAL");
  query.exec("PRAGMA synchronous = NORMAL");
  if (!query.exec(tableDefinition)) {
    qWarning("Create table[%s] failed: %s", qPrintable(tableDefinition), qPrintable(query.lastError().text()));
    return false;  // db and query will destroyed when out of scope
  }
  qWarning("Table[%s] create succeed", qPrintable(tableName));
  return true;
}

QString DbManager::GetRmvCmd(DROP_OR_DELETE dropOrDelete) {
  switch (dropOrDelete) {
    case DROP_OR_DELETE::DROP:
      return DROP_TABLE_TEMPLATE;
    case DROP_OR_DELETE::DELETE:
      return DELETE_TABLE_TEMPLATE;
    default:
      return "";
  }
}

int DbManager::RmvTable(const QString& tableNameRegexPattern, DROP_OR_DELETE dropOrDelete, bool isFullMatch) {
  if (!mIsValid) {
    qWarning("invalid cannot drop table");
    return -1;
  }

  const QRegularExpression regex{isFullMatch ? '^' + tableNameRegexPattern + '$' : tableNameRegexPattern};
  if (!regex.isValid()) {
    qWarning("regex[%s] is invalid", qPrintable(tableNameRegexPattern));
    return -1;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return false;
  }

  const QString rmvCmdTemplate{GetRmvCmd(dropOrDelete)};
  if (rmvCmdTemplate.isEmpty()) {
    qWarning("rmvCmdTemplate empty. mode[%d] invalid", (int)dropOrDelete);
    return false;
  }

  int succeedDropCnt = 0;
  QSqlQuery query(db);
  const QStringList& allTables = db.tables();
  for (const QString& tableName : allTables) {
    if (!IsMatch(tableName, regex)) {
      continue;
    }
    if (!query.exec(rmvCmdTemplate.arg(tableName))) {
      qWarning("Drop table[%s] failed: %s",  //
               qPrintable(tableName), qPrintable(query.lastError().text()));
      db.rollback();
      return -1;
    }
    ++succeedDropCnt;
  }
  qDebug("Drop %d table(s) from %d table(s) by user specified pattern[%s]",  //
         succeedDropCnt, allTables.size(), qPrintable(tableNameRegexPattern));
  return succeedDropCnt;
}

bool DbManager::DeleteDatabase() {
  if (!mIsValid) {
    qWarning("invalid cannot drop database");
    return false;
  }
  ReleaseConnection();
  if (QFile{mDbName}.exists()) {
    if (!QFile(mDbName).moveToTrash()) {
      qWarning("database[%s] move to trash failed", qPrintable(mDbName));
      return false;
    }
  }
  mIsValid = false;
  return true;
}
