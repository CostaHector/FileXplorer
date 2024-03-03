#ifndef RatingSqlTableModel_H
#define RatingSqlTableModel_H

#include <QPixmap>
#include <QSqlTableModel>

class RatingSqlTableModel : public QSqlTableModel {
 public:
  explicit RatingSqlTableModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase()) : QSqlTableModel{parent, db} {
    setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

#endif  // RatingSqlTableModel_H
