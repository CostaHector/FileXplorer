#ifndef VIDMODEL_H
#define VIDMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include "qitemselectionmodel.h"

class VidModel : public QAbstractListModel {
 public:
  explicit VidModel(QObject* parent = nullptr);

  int appendAPath(const QString& path);
  int appendRows(const QStringList& lst);

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_vids.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return 1; }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Vertical) {
        return section + 1;
      }
    }
    return QAbstractListModel::headerData(section, orientation, role);
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role);

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == 2) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  QString filePath(const QModelIndex& index) const {
    if (not index.isValid()) {
      return "";
    }
    return m_vids[index.row()];  // Todo. May some int bool here
  }

  void clear();

  void whenFilesDeleted(const QItemSelection& selections);

  void updatePlayableForeground();

 private:
  QString m_rootPath;
  QList<QString> m_vids;
};

#endif  // VIDMODEL_H
