#include "SubscribeDatabase.h"
#include <QAbstractItemView>
#include <QMainWindow>

auto InitDataBase() -> bool {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

auto InitATable() -> bool {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }

  if (con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] already exist in database[%s]", TABLE_NAME.toStdString().c_str(), con.databaseName().toStdString().c_str());
    return true;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = QString(
                                      "CREATE TABLE IF NOT EXISTS `%1`("
                                      "   `%2` TEXT NOT NULL,"
                                      "   `%3` INT,"
                                      "   `%4` TEXT NOT NULL,"
                                      "   `%5` TEXT,"
                                      "   `%6` TEXT,"
                                      "   `%7` TEXT,"
                                      "   `%8` INT,"
                                      "   `%9` TEXT,"
                                      "   `%10` TEXT NOT NULL,"
                                      "   `%11` TEXT,"
                                      "    PRIMARY KEY (%9, %10, "
                                      "                %2, %4,"
                                      "                %6, %7)"
                                      "    );")
                                      .arg(TABLE_NAME)
                                      .arg(DB_HEADER_KEY::Name)
                                      .arg(DB_HEADER_KEY::Size)
                                      .arg(DB_HEADER_KEY::Type)
                                      .arg(DB_HEADER_KEY::DateModified)
                                      .arg(DB_HEADER_KEY::Performers)
                                      .arg(DB_HEADER_KEY::Tags)
                                      .arg(DB_HEADER_KEY::Rate)
                                      .arg(DB_HEADER_KEY::Driver)
                                      .arg(DB_HEADER_KEY::Prepath)
                                      .arg(DB_HEADER_KEY::Extra);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", TABLE_NAME.toStdString().c_str());
    return false;
  }
  return true;
}

auto InsertIntoTable(const QString& path) -> bool {
  if (not QFileInfo(path).isDir()) {
    qDebug("input is not a path[%s]", path.toStdString().c_str());
    return false;
  }
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] not exist. Insert Abort", TABLE_NAME.toStdString().c_str());
    return false;
  }

  QSqlQuery insertTableQuery(con);
  const QString& insertTemplate = QString("INSERT INTO `%1` (%2) VALUES").arg(TABLE_NAME).arg(DB_HEADER_KEY::DB_HEADER.join(",")) +
                                  QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", \"%6\", %7, \"%8\", \"%9\", \"%10\");");

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QDirIterator it(path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  const QString CURRENT_DRIVE_LETTER = QStorageInfo(path).rootPath();
  while (it.hasNext()) {
    it.next();
    QFileInfo fi = it.fileInfo();
    const QString currentInsert = insertTemplate.arg(fi.fileName())
                                      .arg(fi.size())
                                      .arg(fi.suffix())
                                      .arg(fi.lastModified().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                                      .arg("")
                                      .arg("")
                                      .arg(0)
                                      .arg(CURRENT_DRIVE_LETTER)
                                      .arg(fi.absolutePath())
                                      .arg("");
    const bool insertResult = insertTableQuery.exec(currentInsert);
    succeedItemCnt += int(insertResult);
    if (not insertResult) {
      qDebug("Insert Error: %s", insertTableQuery.lastError().databaseText().toStdString().c_str());
    }
    ++totalItemCnt;
  }
  qDebug("%d/%d item(s) add succeed.", succeedItemCnt, totalItemCnt);
  insertTableQuery.finish();
  return true;
}

auto SubscribeDatabase::GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString {
  QStringList clauseLst;
  for (QAction* act : selectByDriveActs) {
    if (act->isChecked()) {
      clauseLst.append(QString("Driver=\"%1\"").arg(act->text()));
    }
  }
  if (clauseLst.isEmpty()) {  // select none driver
    return "0";
  }
  if (clauseLst.size() == selectByDriveActs.size()) {  // select all driver
    return "";
  }
  return clauseLst.join(" OR ");
}
