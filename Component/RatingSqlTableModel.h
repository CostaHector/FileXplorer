#ifndef RatingSqlTableModel_H
#define RatingSqlTableModel_H

#include <QSqlTableModel>
#include <QPixmap>

class RatingSqlTableModel : public QSqlTableModel {
 public:
  explicit RatingSqlTableModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase()) : QSqlTableModel{parent, db} {}
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

#endif  // RatingSqlTableModel_H
