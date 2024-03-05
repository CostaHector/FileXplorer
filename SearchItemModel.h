#ifndef SEARCHITEMMODEL_H
#define SEARCHITEMMODEL_H

#include <QDir>
#include <QHash>
#include <QStandardItemModel>
#include "PublicVariable.h"
#include <QFileIconProvider>
struct FilePropertyTemp {
  char name[256];
  qint64 size;
  char type[10];
  qint64 createdTime;
  char relPath[256];
};

QString DirFilter2String(const QDir::Filters& dirFilters);
QDir::Filters String2DirFilters(const QString& dirFilterStr);

class SearchItemModel : public QStandardItemModel {
 public:
  explicit SearchItemModel(QObject* parent = nullptr);
  ~SearchItemModel();

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role != Qt::DisplayRole) {
      return QVariant();
    }
    if (orientation == Qt::Horizontal) {
      return SearchKey::EXPLORER_COLUMNS_TITLE[section];
    }
    return section + 1;
  }

  bool ChangeRootPathOrFilter(const QString& rootPath, const QDir::Filters& dirFilters, const QStringList& fileTypeFilter, bool forceUpdate = false);
  QVariant data(const QModelIndex& index, int role) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override { return indexData_.contains(m_Key) ? indexData_[m_Key].size() : 0; }
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  auto rootDirectory(const QString& placeHolder = "" /* no use */) -> QDir { return QDir(rootPath()); }

  auto rootPath(const QString& placeHolder = "" /* no use */) -> QString { return m_rootPath; }

  auto relPath(QModelIndex curIndex) -> QString {
    QModelIndex relPathIndex = index(curIndex.row(), SearchKey::RELPATH_COLUMN, curIndex.parent());
    return data(relPathIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto fileName(QModelIndex curIndex) -> QString {
    QModelIndex nameIndex = index(curIndex.row(), SearchKey::NAME_COLUMN, curIndex.parent());
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto filePath(QModelIndex curIndex) -> QString { return QDir(m_rootPath).absoluteFilePath(relPath(curIndex)); }

  auto fileInfo(QModelIndex curIndex) -> QFileInfo { return QFileInfo(filePath(curIndex)); }

 private:
  // rootPath, typeFilter
  QHash<QString, QList<FilePropertyTemp>> indexData_;
  QString m_Key;
  QString m_rootPath;
  QFileIconProvider m_icon_provider;
};

#endif  // SEARCHITEMMODEL_H
