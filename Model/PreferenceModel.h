#ifndef PREFERENCEMODEL_H
#define PREFERENCEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include "PublicVariable.h"

struct AlertItem {
 public:
  explicit AlertItem(int alarmID_, const KV* kv_, QString note_);
  // QList<AlertItem> will call copy constructor, m_kv cannot be default copied
  static QIcon GetAlertIcon(const AlertItem& alert);
  static const QStringList ALERT_TABLE_HEADER;

  void setValue(const QVariant& newValue);
  QString defaultValue2Str() const;
  QString value2Str() const;

 private:
  QVariant getStoredValue();
  bool isStoredValuePass() const;
  bool isPass(const QVariant& userInput) const;

  const KV* m_kv;

 public:
  int alarmID;
  QString name;
  QVariant value;
  QString note;
  bool checkRes;
};

class PreferenceModel : public QAbstractTableModel {
 public:
  explicit PreferenceModel(QObject* parent = nullptr);

  QModelIndex setRootPath(const QString& /*path*/);

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_alerts.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return AlertItem::ALERT_TABLE_HEADER.size(); }
  int failCount() const;
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

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
    return m_alerts[index.row()].value2Str();
  }

 private:
  QList<AlertItem> m_alerts;
};

#endif  // PREFERENCEMODEL_H
