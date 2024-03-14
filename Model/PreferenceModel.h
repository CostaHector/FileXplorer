#ifndef PREFERENCEMODEL_H
#define PREFERENCEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

struct AlertItem {
 public:
  int alarmID;
  QString name;
  QString value;
  QString note;

  static QIcon GetColor(const AlertItem& alert);

  static const QStringList ALERT_TABLE_HEADER;
};

class PreferenceModel : public QAbstractTableModel {
 public:
  explicit PreferenceModel(QObject* parent = nullptr);
  void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

  QModelIndex setRootPath(const QString& /*path*/);

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_alerts.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return AlertItem::ALERT_TABLE_HEADER.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return AlertItem::ALERT_TABLE_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

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
    return m_alerts[index.row()].value;
  }

 private:
  QList<AlertItem> m_alerts;
};

#endif  // PREFERENCEMODEL_H
