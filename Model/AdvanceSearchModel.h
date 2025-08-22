#ifndef ADVANCESEARCHMODEL_H
#define ADVANCESEARCHMODEL_H

#include "QAbstractTableModelPub.h"
#include "SelectionsRangeHelper.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QSet>
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
  const QString& rootPath() const { return m_rootPath; }
  QDir rootDirectory(const QString& /*placeHolder*/ = "") const {  //
    return QDir{rootPath()};
  }

  void updateSearchResultList();
  bool checkPathNeed(const QString& path, const bool queryWhenSearchUnderLargeDirectory = true);
  void initRootPath(const QString& path);
  void setRootPath(const QString& path);
  void forceRefresh() { setRootPath(m_rootPath); }

  void initFilter(QDir::Filters initialFilters);
  void setFilter(QDir::Filters newFilters);
  void setRootPathAndFilter(const QString& path, QDir::Filters filters = QDir::Files);

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

  void ClearCopyAndCutDict() {
    ClearCutDict();
    ClearCopiedDict();
  }
  void ClearCutDict() { mCutIndexes.clear(); }
  void ClearCopiedDict() { mCopyIndexes.clear();}
  void CutSomething(const QModelIndexList& cutIndexes) {
    auto beRngLst = mCutIndexes.GetTopBottomRange();
    ClearCopyAndCutDict();
    mCutIndexes.Set(rootPath(), cutIndexes);
    beRngLst += mCutIndexes.GetTopBottomRange();
    EmitDecorationRoleChange(beRngLst);
  }
  void CopiedSomething(const QModelIndexList& copiedIndexes) {
    auto beRngLst = mCutIndexes.GetTopBottomRange();
    ClearCopyAndCutDict();
    mCopyIndexes.Set(rootPath(), copiedIndexes);
    beRngLst += mCutIndexes.GetTopBottomRange();
    EmitDecorationRoleChange(beRngLst);
  }

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
    const auto& item = m_itemsLst[row];
    return item.relPath + item.name;
  }
  QString GetARootPath(const int& row) const {
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_rootPath + '/' + m_itemsLst[row].relPath;
  }
  QString GetASelection(const int& row) const {
    if (row < 0 || row >= m_itemsLst.size()) {
      return {};
    }
    return m_itemsLst[row].name;
  }

private:
  void EmitDecorationRoleChange(const SelectionsRangeHelper::ROW_RANGES_LST& beRngLst) {
    foreach (auto beRng, beRngLst) {
      if (beRng.first.isValid() && beRng.first.row() < rowCount() && beRng.second.isValid() && beRng.second.row() < rowCount()) {
        emit dataChanged(beRng.first, beRng.second, {Qt::DecorationRole});
      }
    }
  }

  QString m_rootPath;
  QList<FileProperty> m_itemsLst;
  QFileIconProvider m_iconProvider;

  QDir::Filters m_filters;
  QDirIterator::IteratorFlags m_iteratorFlags;

  SelectionsRangeHelper mCutIndexes, mCopyIndexes;
  QSet<QModelIndex> m_disableList;  // QFileSystemModel: only setNameFilter will effect this
  // SearchModel: both setNameFilter and contents will effect this
  QSet<QModelIndex> m_recycleSet;
  static const QStringList HORIZONTAL_HEADER_NAMES;
};

#endif  // ADVANCESEARCHMODEL_H
