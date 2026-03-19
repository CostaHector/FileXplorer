#ifndef CONFIGSMODEL_H
#define CONFIGSMODEL_H

#include <QAbstractTableModel>
class QSettings;
class KV;

class ConfigsModel : public QAbstractTableModel {
public:
  enum DataType {
    NAME = 0,
    INITIAL_VALUE = 1,
    CURRENT_VALUE = 2,
  };

  using QAbstractTableModel::QAbstractTableModel;

  int rowCount(const QModelIndex& /*parent*/ = {}) const override;
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return CONFIGS_TABLE_HEADER.size(); }
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
    if (index.column() == CURRENT_VALUE) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  QString filePath(const QModelIndex& index) const;
  static bool isCfgPass(const QSettings& curCfg, const KV& record);

private:
  static const QStringList CONFIGS_TABLE_HEADER;
};

#endif  // CONFIGSMODEL_H
