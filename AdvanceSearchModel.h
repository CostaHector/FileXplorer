#ifndef ADVANCESEARCHMODEL_H
#define ADVANCESEARCHMODEL_H

#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QSortFilterProxyModel>
#include <QWidget>

struct FileProperty {
  QString name;
  qint64 size;
  QString type;
  QDateTime modifiedDate;
  QString relPath;
};

class AdvanceSearchModel : public QAbstractTableModel {
 public:
  explicit AdvanceSearchModel(QObject* parent = nullptr);

  auto filter() const -> QDir::Filters { return m_filters; }
  auto _updatePlanetList() -> void;

  inline QString rootPath() const { return m_rootPath; }
  auto checkPathNeed(const QString& path) const -> bool;
  auto setRootPath(const QString& path) -> void;

  auto initFilter(QDir::Filters initialFilters) -> void;
  auto setFilter(QDir::Filters newFilters) -> void;
  auto setRootPathAndFilter(const QString& path, QDir::Filters filters = QDir::Files) -> void;

  QDirIterator::IteratorFlag bool2IteratorFlag(const bool isIncludeEnabled) const;
  void initIteratorFlag(QDirIterator::IteratorFlag initialFlags);
  void setIteratorFlag(QDirIterator::IteratorFlag newFlags) {
    initIteratorFlag(newFlags);
    _updatePlanetList();
  }

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_planetList.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return HORIZONTAL_HEADER_NAMES.size(); }
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
  auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override;

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

 private:
  QString m_rootPath;
  QList<FileProperty> m_planetList;
  QFileIconProvider m_iconProvider;
  QDir::Filters m_filters;
  QDirIterator::IteratorFlags m_iteratorFlags;

  QHash<QString, QModelIndexList> m_copiedMap;
  QHash<QString, QModelIndexList> m_cutMap;
  QSet<QModelIndex> m_disableList;  // QFileSystemModel: only setNameFilter will effect this
                                    // SearchModel: both setNameFilter and contents will effect this
  static const QStringList HORIZONTAL_HEADER_NAMES;
};

#endif  // ADVANCESEARCHMODEL_H
