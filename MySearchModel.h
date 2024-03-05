#ifndef MYSEARCHMODEL_H
#define MYSEARCHMODEL_H

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

class MySearchModel : public QAbstractTableModel {
 public:
  explicit MySearchModel(QObject* parent = nullptr) : QAbstractTableModel(parent){};

  auto filter() -> QDir::Filters { return m_filters; }
  auto _updatePlanetList() -> void;

  inline QString rootPath() const{return m_rootPath;}
  auto checkPathNeed(const QString& path) const -> bool;
  auto setRootPath(const QString& path) -> void;
  auto setFilter(QDir::Filters filters = QDir::Files) -> void;
  auto setRootPathAndFilter(const QString& path, QDir::Filters filters = QDir::Files) -> void;

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_planetList.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return HORIZONTAL_HEADER_NAMES.size(); }
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
  auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override;

  auto BindProxyModel(QSortFilterProxyModel* proxyModel) -> void { m_proxyModel = proxyModel; }
  auto setNameFilterDisables(bool enable) -> void { qDebug("Todo. setNameFilterDisables not supported in MySearchModel."); }

 private:
  QString m_rootPath;
  QList<FileProperty> m_planetList;
  QFileIconProvider m_iconProvider;
  QDir::Filters m_filters = QDir::Files;
  QSortFilterProxyModel* m_proxyModel = nullptr;

  static const QStringList HORIZONTAL_HEADER_NAMES;
};

#endif  // MYSEARCHMODEL_H
