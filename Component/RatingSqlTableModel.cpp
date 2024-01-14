#include "RatingSqlTableModel.h"
#include "Tools/PerformerJsonFileHelper.h"

QVariant RatingSqlTableModel::data(const QModelIndex& index, int role) const {
  static const QPixmap PERFORMER_SCORE_BOARD[] = {QPixmap(":/themes/RATE0").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE1").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE2").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE3").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE4").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE5").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE6").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE7").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE8").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE9").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                                  QPixmap(":/themes/RATE10").scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation)};

  if (index.column() == PERFORMER_DB_HEADER_KEY::Rate_INDEX) {
    //    return QIcon(":/themes/RATE" + QSqlTableModel::data(index, Qt::DisplayRole).toString());
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    if (role == Qt::DecorationRole) {
      return PERFORMER_SCORE_BOARD[(sc > 10) ? 10 : (sc < 0 ? 0 : sc)];
    }
  }
  return QSqlTableModel::data(index, role);
}
