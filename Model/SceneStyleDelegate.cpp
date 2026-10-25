#include "SceneStyleDelegate.h"
#include "PublicMacro.h"
#include "SceneInfo.h"
#include "RateHelper.h"
#include "ColorTools.h"
#include "DataFormatter.h"
#include "VideoTierTool.h"

#include <QApplication>
#include <QPainter>

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
}

void SceneStyleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  CHECK_NULLPTR_RETURN_VOID(painter);

  // 1. Icon and Name
  {
    QStyledItemDelegate::paint(painter, option, index);
    // 1. Don't Draw Icon and Name
    // QStyleOptionViewItem bgOpt = option;
    // initStyleOption(&bgOpt, index);           // 保证 features、state 等字段完整
    // bgOpt.icon = QIcon();
    // bgOpt.features &= ~QStyleOptionViewItem::HasDecoration;
    // bgOpt.text.clear();
    // bgOpt.features &= ~QStyleOptionViewItem::HasDisplay;
    // QStyle* style = bgOpt.widget ? bgOpt.widget->style() : QApplication::style();
    // style->drawControl(QStyle::CE_ItemViewItem, &bgOpt, painter, bgOpt.widget);
  }

  // 2. Meta Info cover image below half
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  if (QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style()) {
    constexpr int META_H = 28;
    const QRect decoRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    const QRect metaRect(decoRect.left(), decoRect.bottom() - META_H + 1, decoRect.width(), META_H);
    drawMeta (painter, metaRect, index);
  }

  // 3. Rating bar
  if (mRateMachine.status() == RatingState::SELECTED_SHOW && index == mRateMachine.curIndex()) {
    drawRatingGrid(painter, GetRatingAreaRect(option.rect), mRateMachine.oldRate(), mRateMachine.newRate());
  }

  // 4. Current rate
  const int rating = index.data(SceneInfo::Role::RATE_ROLE).toInt();
  drawCurrentRateSquare(painter, option.rect, rating);
}

void SceneStyleDelegate::drawCurrentRateSquare(QPainter* painter, QRect currentRateTextRect, int rating) {
  CHECK_NULLPTR_RETURN_VOID(painter);
  currentRateTextRect.setHeight(RateHelper::RATING_BAR_HEIGHT);
  currentRateTextRect.setWidth(RateHelper::RATING_BAR_X);

  painter->save();

  QColor bg{VideoTierTool::GetVideoTierColorRgb(VideoTierTool::MovieRate2VideoTierE(rating))};
  bg.setAlpha(210);
  painter->fillRect(currentRateTextRect, bg);

  painter->setPen(Qt::white);
  painter->drawText(currentRateTextRect, Qt::AlignCenter, QString::number(rating));

  painter->restore();
}

void SceneStyleDelegate::drawRatingGrid(QPainter* painter, const QRect& rect, int rating, int hoverRating) {
  CHECK_NULLPTR_RETURN_VOID(painter);
  // 绘制半透明背景层
  painter->save();

  painter->fillRect(rect, ColorTools::GRAY_AND_HALF_TRANS); // gray and half transparent
  const int EACH_LINE_WIDTH{rect.width() / 10};

  // 黄色 评分条
  QRect hoverRectBar{rect};
  hoverRectBar.setWidth(hoverRating * EACH_LINE_WIDTH);
  painter->setBrush(ColorTools::YELLOW_COLOR);
  painter->fillRect(hoverRectBar, ColorTools::YELLOW_COLOR);

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

  painter->restore();
}

void SceneStyleDelegate::onSceneClicked(const QModelIndex& nowInd, const QRect& visualRect, const QPoint& clickedPnt) {
  mRateMachine.DoStateTransition(*this, nowInd, visualRect, clickedPnt);
}

QRect SceneStyleDelegate::GetRatingAreaRect(QRect gridVisualRect) {
  gridVisualRect.setLeft(gridVisualRect.left() + RateHelper::RATING_BAR_X);
  gridVisualRect.setHeight(RateHelper::RATING_BAR_HEIGHT);
  return gridVisualRect;
}

void SceneStyleDelegate::drawMeta(QPainter* painter, const QRect& rect, const QModelIndex& index) {
  painter->save();
  painter->fillRect(rect, ColorTools::GRAY_AND_HALF_TRANS);

  const int duration = index.data(SceneInfo::Role::DURATION_ROLE).toInt();
  painter->setPen(QColor(0xCC, 0xCC, 0xCC));
  painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, DataFormatter::formatDurationISO(duration));

  const qint64 vidSize = index.data(SceneInfo::Role::VID_SIZE_ROLE).toLongLong();
  painter->setPen(QColor(0x99, 0x99, 0x99));
  painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, DataFormatter::formatFileSizeGMKB(vidSize));

  painter->restore();
}