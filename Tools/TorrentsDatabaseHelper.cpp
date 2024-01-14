#include "TorrentsDatabaseHelper.h"

TorrentsDatabaseHelper::TorrentsDatabaseHelper() {}

QString TorrentsDatabaseHelper::CreatePerformerTableSQL(const QString& tableName) {
  return QString(
             "CREATE TABLE IF NOT EXISTS `%1`("
             "   `%2` NCHAR(256) NOT NULL,"
             "   `%3` INT DEFAULT 0,"
             "   `%5` CHAR(10),"
             "   `%6` INT,"
             "   `%7` CHAR(64) default \"\","
             "   `%8` NCHAR(256) NOT NULL,"
             "    PRIMARY KEY (%2, %8)"
             "    );")
      .arg(tableName)
      .arg(TORRENTS_DB_HEADER_KEY::Name)
      .arg(TORRENTS_DB_HEADER_KEY::Size)
      .arg(TORRENTS_DB_HEADER_KEY::Type)
      .arg(TORRENTS_DB_HEADER_KEY::DateModified)
      .arg(TORRENTS_DB_HEADER_KEY::MD5)
      .arg(TORRENTS_DB_HEADER_KEY::PREPATH);
}
