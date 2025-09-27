#ifndef HARMODEL_H
#define HARMODEL_H

#include "QAbstractTableModelPub.h"
#include "HarFiles.h"
#include <QItemSelectionModel>
#include <QFileIconProvider>

namespace HarFilesMocker {
inline HarFiles& mockHarFiles() {
  static HarFiles harFiles;
  return harFiles;
}
}  // namespace HarFilesMocker

class HarModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;

  int setRootPath(const QString& harFileAbsPath);

  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return mHarParser.size(); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return HAR_VERTICAL_HEAD.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
        return HAR_VERTICAL_HEAD[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& /*index*/) const override {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  int SaveToLocal(QString dstRootpath = "", const QList<int>& selectedRows = {});
  const HAR_FILE_ITEM& GetHarEntryItem(const int rowIndex) const;

 private:
  HarFiles mHarParser;
  QFileIconProvider m_iconProvider;
  static const QStringList HAR_VERTICAL_HEAD;
};

#endif  // HARMODEL_H
