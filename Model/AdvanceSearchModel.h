﻿#ifndef ADVANCESEARCHMODEL_H
#define ADVANCESEARCHMODEL_H

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QSet>
#include "QAbstractTableModelPub.h"
#include <QDataStream>

struct FileProperty {
  QString name;
  qint64 size;
  QString type;
  QDateTime modifiedDate;
  QString relPath;
  static QString mRootPath;
};

// QDataStream& operator<<(QDataStream& dsIn, const FileProperty& fp) {
//   dsIn << fp.name << fp.size << fp.type << fp.modifiedDate << fp.relPath;
//   return dsIn;
// }
// QDataStream& operator>>(QDataStream& dsOut, FileProperty& fp) {
//   dsOut >> fp.name >> fp.size >> fp.type >> fp.modifiedDate >> fp.relPath;
//   return dsOut;
// }

class AdvanceSearchModel : public QAbstractTableModelPub {
 public:
  explicit AdvanceSearchModel(QObject* parent = nullptr);

  QDir::Filters filter() const { return m_filters; }
  QString rootPath() const { return m_rootPath; }
  QDir rootDirectory(const QString& /*placeHolder*/ = "") const {  //
    return QDir(rootPath());
  }

  void updateSearchResultList();
  bool checkPathNeed(const QString& path, const bool queryWhenSearchUnderLargeDirectory = true);
  auto initRootPath(const QString& path) -> void;
  auto setRootPath(const QString& path) -> void;
  auto forceRefresh() -> void { setRootPath(m_rootPath); }

  auto initFilter(QDir::Filters initialFilters) -> void;
  auto setFilter(QDir::Filters newFilters) -> void;
  auto setRootPathAndFilter(const QString& path, QDir::Filters filters = QDir::Files) -> void;

  QDirIterator::IteratorFlag bool2IteratorFlag(const bool isIncludeEnabled) const;
  void initIteratorFlag(QDirIterator::IteratorFlag initialFlags);
  void setIteratorFlag(QDirIterator::IteratorFlag newFlags) {
    initIteratorFlag(newFlags);
    updateSearchResultList();
  }

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_itemsLst.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return HORIZONTAL_HEADER_NAMES.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void setNameFilterDisables(bool enable) = delete;
  // call setNameFilterDisables or proxy model instead
  void initNameFilterDisables(bool enable) = delete;

  void ClearCopyAndCutDict();
  void ClearCutDict();
  void ClearCopiedDict();
  void CutSomething(const QModelIndexList& cutIndexes, bool appendMode = false);
  void CopiedSomething(const QModelIndexList& copiedIndexes, bool appendMode = false);

  void appendDisable(const QModelIndex& ind);
  void removeDisable(const QModelIndex& ind);
  void clearDisables();

  void RecycleSomething(const QSet<QModelIndex>& recycleIndexes);
  void ClearRecycle();

  QString absolutePath(const QModelIndex& curIndex) const {  //
    const int row = curIndex.row();
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_rootPath + '/' + m_itemsLst[row].relPath;
  }

  QString fileName(const QModelIndex& curIndex) const {  //
    const int row = curIndex.row();
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_itemsLst[row].name;
  }

  QString filePath(const QModelIndex& curIndex) const {  //
    const int row = curIndex.row();
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_rootPath + '/' + m_itemsLst[row].relPath + m_itemsLst[row].name;
  }

  QFileInfo fileInfo(const QModelIndex& curIndex) const {  //
    return QFileInfo{filePath(curIndex)};
  }

  QString fullInfo(const QModelIndex& curIndex) const {  //
    const int row = curIndex.row();
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_itemsLst[row].name                                 //
           + '\t'                                               //
           + QString::number(m_itemsLst[row].size) + "Byte(s)"  //
           + '\t'                                               //
           + m_rootPath + '/' + m_itemsLst[row].relPath;
  }

  QString GetARelSelection(const int& row) const {
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    //  info.relSelections.append();
    const auto& item = m_itemsLst[row];
    return item.relPath + item.name;
  }
  QString GetARootPath(const int& row) const {
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    //  info.rootPaths.append();
    return m_rootPath + '/' + m_itemsLst[row].relPath;
  }
  QString GetASelection(const int& row) const {
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    //  info.selections.append();
    return m_itemsLst[row].name;
  }

 private:
  QString m_rootPath;
  QList<FileProperty> m_itemsLst;
  mutable QHash<QString, QIcon> m_ext2Icon;
  QFileIconProvider m_iconProvider;

  QDir::Filters m_filters;
  QDirIterator::IteratorFlags m_iteratorFlags;

  QHash<QString, QModelIndexList> m_copiedMap;
  QHash<QString, QModelIndexList> m_cutMap;
  QSet<QModelIndex> m_disableList;  // QFileSystemModel: only setNameFilter will effect this
                                    // SearchModel: both setNameFilter and contents will effect this
  QSet<QModelIndex> m_recycleSet;
  static const QStringList HORIZONTAL_HEADER_NAMES;
};

#endif  // ADVANCESEARCHMODEL_H
