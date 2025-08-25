#include "RatingSqlTableModel.h"
#include "TableFields.h"
#include <QPainter>
#include <QPixmap>

static const int MAX_RATE = 10;

QPixmap GetRatePixmap(int r, bool hasBorder = false) {
  if (r < 0 || r > MAX_RATE) {
    qDebug("rate[%d] out bound", r);
    return {};
  }
  static constexpr int WIDTH = 100, HEIGHT = (int)(WIDTH * 0.618);
  QPixmap mp{WIDTH, HEIGHT};
  int originWith = WIDTH * r / 10;
  static constexpr QColor OPAGUE{0, 0, 0, 0};
  static constexpr QColor STD_ORANGE{255, 165, 0, 255};
  mp.fill(OPAGUE); // opague
  QPainter painter{&mp};
  painter.setPen(STD_ORANGE); // standard orange
  painter.setBrush(STD_ORANGE);
  painter.drawRect(0, 0, originWith, HEIGHT);
  if (hasBorder) {
    painter.setPen(QColor{0, 0, 0, 255}); // standard black
    painter.setBrush(OPAGUE);
    painter.drawRect(0, 0, WIDTH - 1, HEIGHT - 1);
  }
  painter.end();
  return mp;
}

QVariant RatingSqlTableModel::data(const QModelIndex& index, int role) const {
  static const QPixmap PERFORMER_SCORE_BOARD[MAX_RATE + 1] = {GetRatePixmap(0), GetRatePixmap(1), GetRatePixmap(2), GetRatePixmap(3), GetRatePixmap(4), GetRatePixmap(5),
                                                              GetRatePixmap(6), GetRatePixmap(7), GetRatePixmap(8), GetRatePixmap(9), GetRatePixmap(10)};
  if (index.column() == PERFORMER_DB_HEADER_KEY::Rate) {
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    if (role == Qt::DecorationRole) {
      return PERFORMER_SCORE_BOARD[(sc > MAX_RATE) ? MAX_RATE : (sc < 0 ? 0 : sc)];
    }
  }
  return QSqlTableModel::data(index, role);
}
