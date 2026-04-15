#ifndef SCENESTYLEDELEGATE_H
#define SCENESTYLEDELEGATE_H

#include <QStyledItemDelegate>
#include "RatingStateMachine.h"

class SceneStyleDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
  QString displayText(const QVariant& value, const QLocale& loc) const override;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  static QRect GetRatingAreaRect(QRect gridVisualRect);
  void onSceneClicked(const QModelIndex& nowInd, const QRect& vRect, const QPoint& clickedPnt);

 signals:
  void cellVisualUpdateRequested(const QModelIndex& ind);

 private:
  // 绘制评分网格
  static void drawRatingGrid(QPainter* painter, const QRect& rect, int rating, int hoverRating = 0);
  static void drawCurrentRateSquare(QPainter* painter, QRect currentRateTextRect, int rating);

  RatingStateMachine mRateMachine;
  static constexpr QColor GRAY_AND_HALF_TRANS{0, 0, 0, 150};
  static constexpr QColor YELLOW_COLOR{255, 200, 0, 220};
};

#endif  // SCENESTYLEDELEGATE_H
