#ifndef COPYITEMPROPERTIESTOCLIPBOARDIF_H
#define COPYITEMPROPERTIESTOCLIPBOARDIF_H

#include <QFileSystemModel>

#include <QListView>
#include <QTableView>
#include <QTreeView>

#include "MyQSqlTableModel.h"

class CopyItemPropertiesToClipboardIF {
 public:
  CopyItemPropertiesToClipboardIF() = default;

  static auto PathCopyTriple(const QStringList& lst, const QString& opName) -> bool;

  static bool on_copySelectedItemFullPath(const QAbstractItemView* view, QFileSystemModel* fileSysModel);
  static bool on_copySelectedItemFullPath(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel);

  static auto on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view, QFileSystemModel* fileSysModel) -> bool;
  static auto on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) -> bool;

  static auto on_copyDirPath(const QAbstractItemView* view, QFileSystemModel* fileSysModel) -> bool;
  static auto on_copyDirPath(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) -> bool;

  static auto on_copyName(const QAbstractItemView* view, QFileSystemModel* fileSysModel) -> bool;
  static auto on_copyName(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) -> bool;
};

#endif  // COPYITEMPROPERTIESTOCLIPBOARDIF_H
