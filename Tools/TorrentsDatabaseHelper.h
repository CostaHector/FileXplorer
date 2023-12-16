#ifndef TORRENTSDATABASEHELPER_H
#define TORRENTSDATABASEHELPER_H

#include <QString>
#include <QStringList>
namespace TORRENTS_DB_HEADER_KEY {
const QString Name = "Name";
const QString Size = "Size";
const QString Type = "Type";
const QString DateModified = "DateModified";
const QString MD5 = "MD5";
const QString PREPATH = "Prepath";
const QStringList HEADERS{Name, Size, Type, DateModified, MD5, PREPATH};
}  // namespace TORRENTS_DB_HEADER_KEY

class TorrentsDatabaseHelper {
 public:
  TorrentsDatabaseHelper();

  static QString CreatePerformerTableSQL(const QString& tableName);
};

#endif  // TORRENTSDATABASEHELPER_H
