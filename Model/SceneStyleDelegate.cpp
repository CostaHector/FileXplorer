#include "SceneStyleDelegate.h"
#include "ScenesListModel.h"
#include <QAbstractItemView>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QGraphicsEffect>

void SceneStyleDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
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
  // 1. plot basic image and text
  QStyledItemDelegate::paint(painter, option, index);
  // 2. only when index = curIndex and display = true. plot rating bar is needed
  if (mRateMachine.status() == RatingState::SELECTED_SHOW && index == mRateMachine.curIndex()) {
    QRect imageRect{GetRealImageVisualRect(index, option.rect)};
    drawRatingGrid(painter, RateHelper::getRatingRect(imageRect), mRateMachine.oldRate(), mRateMachine.newRate());
  }
}

void SceneStyleDelegate::drawRatingGrid(QPainter* painter, const QRect& rect, int rating, int hoverRating) const {
  painter->save();

  // 绘制半透明背景层
  painter->fillRect(rect, QColor{0, 0, 0, 150}); // gray and half transparent

  int gridHeight = rect.height() - 10;
  int gridTop = rect.top() + 5;

  int hoverRateWidth = rect.width() * hoverRating / RateHelper::MOVIE_RATE_VALUE::MAX_V;
  const QRect beforeHoverRect(rect.left(), gridTop, hoverRateWidth, gridHeight);

  // 悬浮点对应评分条
  const QColor beforeHoverGridColor{QColor{255, 200, 0, 220}}; // yellow
  painter->setBrush(beforeHoverGridColor);
  painter->setPen(QPen(Qt::darkGray, 1));
  painter->drawRect(beforeHoverRect);

  // 均匀分布9条垂直线，从左侧开始
  const int leftOfRect = rect.left();
  const int rectWidth = rect.width();

  painter->setPen(Qt::lightGray);
  painter->drawLines(QVector<QLine>{QLine(leftOfRect + rectWidth * 1 / 10, gridTop, leftOfRect + rectWidth * 1 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 2 / 10, gridTop, leftOfRect + rectWidth * 2 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 3 / 10, gridTop, leftOfRect + rectWidth * 3 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 4 / 10, gridTop, leftOfRect + rectWidth * 4 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 5 / 10, gridTop, leftOfRect + rectWidth * 5 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 6 / 10, gridTop, leftOfRect + rectWidth * 6 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 7 / 10, gridTop, leftOfRect + rectWidth * 7 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 8 / 10, gridTop, leftOfRect + rectWidth * 8 / 10, gridTop + gridHeight),
                                    QLine(leftOfRect + rectWidth * 9 / 10, gridTop, leftOfRect + rectWidth * 9 / 10, gridTop + gridHeight)});
  // 绘制数字
  const int EACH_LINE_DISTANCE = rect.width() / 10;
  painter->drawText(QRect(leftOfRect + rectWidth * 1 / 10, gridTop + 2, EACH_LINE_DISTANCE, 14), Qt::AlignCenter, "2");
  painter->drawText(QRect(leftOfRect + rectWidth * 3 / 10, gridTop + 2, EACH_LINE_DISTANCE, 14), Qt::AlignCenter, "4");
  painter->drawText(QRect(leftOfRect + rectWidth * 5 / 10, gridTop + 2, EACH_LINE_DISTANCE, 14), Qt::AlignCenter, "6");
  painter->drawText(QRect(leftOfRect + rectWidth * 7 / 10, gridTop + 2, EACH_LINE_DISTANCE, 14), Qt::AlignCenter, "8");

  painter->setPen(Qt::white);
  painter->drawText(beforeHoverRect, Qt::AlignCenter, QString::asprintf("%d(%d)", hoverRating, rating));

  painter->restore();
}

void SceneStyleDelegate::onSceneClicked(const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt) {
  mRateMachine.DoStateTransition(*this, nowInd, visualRect, clickedPnt);
}

QRect SceneStyleDelegate::GetRealImageVisualRect(const QModelIndex& index, QRect gridVisualRect) const {
  static QStyleOptionViewItem optionBkp;
  QStyledItemDelegate::initStyleOption(&optionBkp, index);
  gridVisualRect.setHeight(optionBkp.decorationSize.height());
  return gridVisualRect;
}
