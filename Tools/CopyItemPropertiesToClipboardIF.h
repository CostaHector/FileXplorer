#ifndef COPYITEMPROPERTIESTOCLIPBOARDIF_H
#define COPYITEMPROPERTIESTOCLIPBOARDIF_H


#include <QListView>
#include <QTableView>
#include <QTreeView>

#include <QAbstractItemView>


class CopyItemPropertiesToClipboardIF {
 public:
  CopyItemPropertiesToClipboardIF() = default;

  static auto PathCopyTriple(const QStringList& lst, const QString& opName) -> bool;

  static bool on_copySelectedItemFullPath(const QAbstractItemView* view);

  static auto on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view) -> bool;

  static auto on_copyDirPath(const QAbstractItemView* view) -> bool;

  static auto on_copyName(const QAbstractItemView* view) -> bool;
};

#endif  // COPYITEMPROPERTIESTOCLIPBOARDIF_H
