#include "RecycleBinHelper.h"
#include "PublicVariable.h"

#include <QSqlError>
#include <QSqlQuery>
#include "Tools/FileSystemItemFilter.h"

auto GetSqlRecycleBinDB() -> QSqlDatabase {
  QSqlDatabase con;
  if (QSqlDatabase::connectionNames().contains("recycle_bin_connection")) {
    con = QSqlDatabase::database("recycle_bin_connection", false);
  } else {
    con = QSqlDatabase::addDatabase("QSQLITE", "recycle_bin_connection");
    con.setDatabaseName(SystemPath::RECYCLE_BIN_DATABASE);
  }
  if (not con.open()) {
    qWarning("Cannot open database[%s]: %s", qPrintable(con.databaseName()), qPrintable(con.lastError().text()));
  }
  return con;
}

QMap<int, int> RecycleBinHelper::s_batchID2Cnt;
int RecycleBinHelper::s_item_id = 0;
const QString RecycleBinHelper::RECYCLE_ITEMS_TABLE = "RECYCLE_ITEMS_TABLE";
const QString RecycleBinHelper::insertTemplate =
    QString("INSERT INTO `%1` (%2) VALUES").arg(RECYCLE_ITEMS_TABLE).arg(RECYCLE_HEADER_KEY::DB_HEADER.join(',')) +
    QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", %6, \"%7\", \"%8\");");

FileOperatorType::BATCH_COMMAND_LIST_TYPE RecycleBinHelper::RecycleABatch(const QStringList& paths, const QStringList& names) {
  // or just stop s_item_id and start after delete;
  if (paths.size() != names.size()) {
    qWarning("paths[%d] and names[%d] list size not equal", paths.size(), names.size());
    return {};
  }
  if (names.isEmpty()) {
    return {};
  }

  auto con = GetSqlRecycleBinDB();
  if (not con.isOpen()) {
    qWarning("Open connection failed");
    return {};
  }

  if (not con.transaction()) {
    qCritical("Failed to start transaction mode");
    return {};
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  cmds.reserve(names.size());

  const auto batchID = UseCurrentBatchID();

  QSet<QString> availTempBatchPath;
  QSqlQuery insertTableQuery(con);
  for (int itemInd = 0; itemInd < names.size(); ++itemInd) {
    const QString& path = paths[itemInd];
    const QString& name = names[itemInd];
    const QFileInfo fi{QDir(path).absoluteFilePath(name)};
    if (not fi.exists()) {
      continue;
    }
    const auto& itemStatic = FileSystemItemFilter::ItemCounter({fi.absoluteFilePath()});
    const QString& tempLocation = ItemMap2Str(path, name, batchID);
    const QString& currentInsert = insertTemplate.arg(name)
                                       .arg(itemStatic.fileSize)
                                       .arg(fi.suffix())
                                       .arg(fi.lastModified().toString("yyyy/MM/dd HH:mm:ss"))
                                       .arg(path)
                                       .arg(batchID)
                                       .arg(tempLocation)
                                       .arg(QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    if (not insertTableQuery.exec(currentInsert)) {
      qWarning("Insert into recycle table failed. items[%s]/[%s], command[%s], msg[%s]", qPrintable(path), qPrintable(name),
               qPrintable(currentInsert), qPrintable(insertTableQuery.lastError().text()));
    } else {
      if (not availTempBatchPath.contains(tempLocation)) {
        cmds.append({"mkpath", "", tempLocation});
        availTempBatchPath.insert(tempLocation);
      }
      cmds.append({"rename", path, name, tempLocation, name});
    }
  }
  insertTableQuery.finish();

  if (!con.commit()) {
    qWarning("Commit recycle cmds failed, all[%d] item(s) will be rollback", names.size());
    //    Notificator::warning("Commit recycle cmds failed", QString("all[%1] item(s) will be rollback").arg(names.size()));
    con.rollback();
  }
  //  m_model->submitAll();
  return cmds;
}

QString RecycleBinHelper::GetRecyleBinRootPath(const QString& path) {
  static QSet<QChar> availableDriver;
#ifdef _WIN32
  if (path.isEmpty()) {
    qWarning("Never pass an empty path here");
    return "";
  }
  const QChar driverLetter = path.front();
  QString recycleRootPath = driverLetter + QString(":/.recyle");
#else
  const QChar driverLetter = '/';
  QString recycleRootPath = SystemPath::desktopPath + '/' + ".recyle";
#endif
  if (availableDriver.contains(driverLetter)) {
    return recycleRootPath;
  }
  const bool pathMkResult = QDir("").mkpath(recycleRootPath);
  if (not pathMkResult) {
    qWarning("Cannot make recycle root path[%s]", qPrintable(recycleRootPath));
    return "";
  }
  availableDriver.insert(driverLetter);
  return recycleRootPath;
}
