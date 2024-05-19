#ifndef RECYCLEBINHELPER_H
#define RECYCLEBINHELPER_H

// for Qt < 5.15
#include <QDir>
#include <QMap>
#include <QSet>
#include <QSqlDatabase>

#include "FileOperatorPub.h"

namespace RECYCLE_HEADER_KEY {
const QString Name = "Name";
const QString Size = "Size";
const QString Type = "Type";
const QString DateModified = "DateModified";
const QString OldLocation = "OldLocation";
const QString DeleteID = "DeleteID";
const QString TempLocation = "TempLocation";
const QString DeleteTime = "DeleteTime";

const QStringList DB_HEADER{Name, Size, Type, DateModified, OldLocation, DeleteID, TempLocation, DeleteTime};
const int DB_NAME_INDEX = DB_HEADER.indexOf(Name);
const int SIZE_INDEX = DB_HEADER.indexOf(Size);
const int OLD_LOCATION_INDEX = DB_HEADER.indexOf(OldLocation);
const int TEMP_LOCATION_INDEX = DB_HEADER.indexOf(TempLocation);
}  // namespace RECYCLE_HEADER_KEY

auto GetSqlRecycleBinDB() -> QSqlDatabase;

class RecycleBinHelper {
 public:
  RecycleBinHelper() {}

  static FileOperatorType::BATCH_COMMAND_LIST_TYPE RecycleABatch(const QStringList& paths, const QStringList& names);

  static QString ItemMap2Str(const QString& path, const QString& name, int id = -1) {
    // Win32 C:/home/to/a file => C:/.recyle/id
    // Linux /home/to/a file=> /home/to/.recyle/id
    if (not QDir(path).exists(name)) {
      return "";
    }
    if (id == -1) {
      id = UseCurrentBatchID();
    }
    if (!s_batchID2Cnt.contains(id)) {
      s_batchID2Cnt[id] = 1;
    } else {
      ++s_batchID2Cnt[id];
    }
    QString recycleRootPath = GetRecyleBinRootPath(path);
    if (recycleRootPath.isEmpty()) {
      return "";
    }
    return recycleRootPath + '/' + QString::number(id);
  }

  static QString GetRecyleBinRootPath(const QString& path);

  static int UseCurrentBatchID() { return s_item_id++; }
  static QMap<int, int> s_batchID2Cnt;
  static int s_item_id;  // batch no. of items being recycled.

  static const QString RECYCLE_ITEMS_TABLE;
  static const QString insertTemplate;
};

#endif  // RECYCLEBINHELPER_H
