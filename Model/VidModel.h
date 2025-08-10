#ifndef VIDMODEL_H
#define VIDMODEL_H

#include "DifferRootFileSystemModel.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QSet>

class VidModel : public DifferRootFileSystemModel {
 public:
  explicit VidModel(QObject* parent = nullptr);

  int appendAPath(const QString& path) override;
  int appendRows(const QStringList& lst) override;

  QString filePath(const QModelIndex& index) const override {
    if (not index.isValid()) {
      qWarning() << "Try to access invalid index" << index;
      return "";
    }
    return m_vids[index.row()];
  }
  QString filePath(const int row) const override {
    if (not(0 <= row and row < rowCount())) {
      qWarning("Try to access row[%d] not in [0, %d)", row, rowCount());
      return "";
    }
    return m_vids[row];
  }

  void clear() override;

  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_vids.size(); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return 1; }

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

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == 2) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  void whenFilesDeleted(const QItemSelection& selections);

  void updatePlayableForeground();

  int getNextAvailableVidUrl(const QUrl& startFrom, const QModelIndexList& notAvailList) const;
  QStringList getToRemoveFileList(const QModelIndexList& toRmvList) const;

 private:
  QString m_rootPath;
  QList<QString> m_vids;
};

#endif  // VIDMODEL_H
