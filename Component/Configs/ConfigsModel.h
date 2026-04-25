#ifndef CONFIGSMODEL_H
#define CONFIGSMODEL_H

#include <QAbstractTableModel>
class QSettings;
class KV;

class ConfigsModel : public QAbstractTableModel {
public:
  enum ColumnE {
    NAME = 0,
    INITIAL_VALUE = 1,
    CURRENT_VALUE = 2,
    VALIDATIDATION_VALUE = 3,
    COLUMNS_COUNT,
    DATA_TYPE_ROLE = Qt::UserRole + 1,
  };

  using QAbstractTableModel::QAbstractTableModel;

  int rowCount(const QModelIndex& /*parent*/ = {}) const override;
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return ColumnE::COLUMNS_COUNT; }
  int failCount() const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return CONFIGS_TABLE_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == EDITABLE_COLUMN) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  bool isPath(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;

  static bool isCfgPass(const QSettings& curCfg, const KV& record);

  static constexpr int EDITABLE_COLUMN = ColumnE::CURRENT_VALUE;
private:
  static const QStringList CONFIGS_TABLE_HEADER;
};

#endif  // CONFIGSMODEL_H
