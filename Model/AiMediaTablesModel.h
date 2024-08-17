#ifndef AIMEDIATABLESMODEL_H
#define AIMEDIATABLESMODEL_H

#include <QAbstractTableModel>
#include "Tools/AIMediaDuplicate.h"

class AiMediaTablesModel : public QAbstractTableModel {
 public:
  AiMediaTablesModel(QObject* parent = nullptr) : QAbstractTableModel{parent} {}

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
  auto rowCount(const QModelIndex& parent = {}) const -> int override { return m_data.size(); }
  auto columnCount(const QModelIndex& parent = {}) const -> int override { return AITABLE_HOR_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QString fileName(const QModelIndex& index) const {
    if (index.isValid() && 0 <= index.row() and index.row() < rowCount()) {
      return m_data[index.row()].tableName;
    }
    qWarning("invalid index");
    return {};
  }
  QStringList fileNames(const QModelIndexList& indx) const {
    QStringList tbls;
    tbls.reserve(indx.size());
    for (const auto& ind : indx) {
      const QString& tblName = fileName(ind);
      if (tblName.isEmpty()) {
        continue;
      }
      tbls.append(tblName);
    }
    return tbls;
  }
  void UpdateData(QList<DupTableModelData> newData);

 private:
  QList<DupTableModelData> m_data;
  static const QStringList AITABLE_HOR_HEADER;
};

#endif  // AIMEDIATABLESMODEL_H
