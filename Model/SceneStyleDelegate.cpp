#include "SceneStyleDelegate.h"
#include "PublicMacro.h"
#include "ScenesListModel.h"
#include <QPainter>

constexpr QColor SceneStyleDelegate::GRAY_AND_HALF_TRANS;
constexpr QColor SceneStyleDelegate::YELLOW_COLOR;

void SceneStyleDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
  CHECK_NULLPTR_RETURN_VOID(option);
  option->decorationPosition = QStyleOptionViewItem::Position::Top;
  option->decorationAlignment = Qt::AlignmentFlag::AlignHCenter;
  option->textElideMode = Qt::TextElideMode::ElideLeft;
  option->displayAlignment = Qt::AlignmentFlag::AlignVCenter;
  option->features |= QStyleOptionViewItem::WrapText;
  QStyledItemDelegate::initStyleOption(option, index);
}

QString SceneStyleDelegate::displayText(const QVariant& value, const QLocale& loc) const {
  return QStyledItemDelegate::displayText(value, loc);
  // const QString& text = value.toString();
  // static constexpr int CHAR_LETTER_CNT = 40;
  // if (text.size() <= CHAR_LETTER_CNT) {
  //   return text;
  // }
  // return text.left(CHAR_LETTER_CNT / 2) + "..." + text.right(CHAR_LETTER_CNT / 2);
}

void SceneStyleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  CHECK_NULLPTR_RETURN_VOID(painter);
  // 1. plot basic image and text
  QStyledItemDelegate::paint(painter, option, index);
  // 2. only when index = curIndex and display = true. plot rating bar is needed
  painter->save();
  if (mRateMachine.status() == RatingState::SELECTED_SHOW && index == mRateMachine.curIndex()) {
    drawRatingGrid(painter, GetRatingAreaRect(option.rect), mRateMachine.oldRate(), mRateMachine.newRate());
  }
  const int rating = index.data(SceneInfo::Role::RATE_ROLE).toInt();
  drawCurrentRateSquare(painter, option.rect, rating);
  painter->restore();
}

void SceneStyleDelegate::drawCurrentRateSquare(QPainter* painter, QRect currentRateTextRect, int rating) {
  CHECK_NULLPTR_RETURN_VOID(painter);
  currentRateTextRect.setHeight(RateHelper::RATING_BAR_HEIGHT);
  currentRateTextRect.setWidth(RateHelper::RATING_BAR_X);
  painter->fillRect(currentRateTextRect, GRAY_AND_HALF_TRANS); // gray and half transparent
  painter->setPen(Qt::white);
  painter->drawText(currentRateTextRect, Qt::AlignCenter, QString::number(rating));
}

void SceneStyleDelegate::drawRatingGrid(QPainter* painter, const QRect& rect, int rating, int hoverRating) {
  CHECK_NULLPTR_RETURN_VOID(painter);
  // 绘制半透明背景层
  painter->fillRect(rect, GRAY_AND_HALF_TRANS); // gray and half transparent
  const int EACH_LINE_WIDTH{rect.width() / 10};

  // 黄色 评分条
  QRect hoverRectBar{rect};
  hoverRectBar.setWidth(hoverRating * EACH_LINE_WIDTH);
  painter->setBrush(YELLOW_COLOR);
  painter->fillRect(hoverRectBar, YELLOW_COLOR);

  const int EACH_BAR_TOP{hoverRectBar.top()};
  const int EACH_BAR_BOTTOM{hoverRectBar.bottom()};
  const int LEFT_X{hoverRectBar.left()};
  const int EACH_BAR_HEIGHT = rect.height();

  // 灰色 均匀绘制9条垂直线，从左侧开始
  painter->setPen(Qt::lightGray);
  painter->drawLines(QVector<QLine>{
      QLine(LEFT_X + 1 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 1 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 2 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 2 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 3 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 3 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 4 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 4 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 5 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 5 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 6 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 6 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 7 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 7 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 8 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 8 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
      QLine(LEFT_X + 9 * EACH_LINE_WIDTH, EACH_BAR_TOP, LEFT_X + 9 * EACH_LINE_WIDTH, EACH_BAR_BOTTOM), //
  });

  // 深灰色 可见候选评分
  painter->setPen(Qt::gray);
  painter->drawText(QRect(LEFT_X + 1 * EACH_LINE_WIDTH, EACH_BAR_TOP, EACH_LINE_WIDTH, EACH_BAR_HEIGHT), Qt::AlignCenter, "2");
  painter->drawText(QRect(LEFT_X + 3 * EACH_LINE_WIDTH, EACH_BAR_TOP, EACH_LINE_WIDTH, EACH_BAR_HEIGHT), Qt::AlignCenter, "4");
  painter->drawText(QRect(LEFT_X + 5 * EACH_LINE_WIDTH, EACH_BAR_TOP, EACH_LINE_WIDTH, EACH_BAR_HEIGHT), Qt::AlignCenter, "6");
  painter->drawText(QRect(LEFT_X + 7 * EACH_LINE_WIDTH, EACH_BAR_TOP, EACH_LINE_WIDTH, EACH_BAR_HEIGHT), Qt::AlignCenter, "8");

  // 白色 悬浮评分/旧评分
  painter->setPen(Qt::white);
  painter->drawText(hoverRectBar, Qt::AlignCenter, QString::asprintf("%d(%d)", hoverRating, rating));
}

void SceneStyleDelegate::onSceneClicked(const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt) {
  mRateMachine.DoStateTransition(*this, nowInd, visualRect, clickedPnt);
}

QRect SceneStyleDelegate::GetRatingAreaRect(QRect gridVisualRect) {
  gridVisualRect.setLeft(gridVisualRect.left() + RateHelper::RATING_BAR_X);
  gridVisualRect.setHeight(RateHelper::RATING_BAR_HEIGHT);
  return gridVisualRect;
}
