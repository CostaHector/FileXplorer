#include "RatingStarsWidget.h"
#include "PublicMacro.h"
#include "RateActions.h"
#include <QMenu>
#include <QToolTip>
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

RatingStarsWidget* RatingStarsWidget::GInstance(RatingStarsWidget* pInitValue, bool bReset) {
  static RatingStarsWidget* pInstance = pInitValue;
  if (bReset) {
    pInstance = pInitValue;
    return nullptr;
  }
  if (pInstance == nullptr) {
    if (pInitValue == nullptr) {
      LOG_W("Instance not init at all");
    } else {
      pInstance = pInitValue;
      LOG_W("Instance should init before here. adjust init position");
    }
  }
  return pInstance;
}

RatingStarsWidget::RatingStarsWidget(QWidget *parent)
  : QWidget(parent), m_starPath{GetStarPath()} {
  auto& rateActionInst = RateActions::GetInst(RateActions::RateRequestFrom::FILE_XPLORER);
  addActions(rateActionInst.GetRateActionsList());

  m_menu = rateActionInst.GetRibbonRateMenu(this);
  setMouseTracking(true);
  setMinimumWidth(20 * starCount()); // each star at least width=20. and reserver for current rate

  connect(&rateActionInst, &RateActions::RateMovieReq, this, &RatingStarsWidget::freshRating);
}

QPainterPath RatingStarsWidget::GetStarPath() const {
  QPainterPath starPath;
  // 构建归一化星形路径
  const int points = 10;
  const double outerR = 1.0;
  const double innerR = 0.4;
  for (int j = 0; j < points; ++j) {
    double r = (j % 2 == 0) ? outerR : innerR;
    double theta = -M_PI/2 + j * 2 * M_PI / points;
    double x = r * cos(theta);
    double y = r * sin(theta);
    if (j == 0) starPath.moveTo(x, y);
    else        starPath.lineTo(x, y);
  }
  starPath.closeSubpath();
  return starPath;
}

void RatingStarsWidget::freshRating(int value) {
  value = clampRate(value);
  if (m_currentRating != value) {
    m_currentRating = value;
    update();
  }
}

void RatingStarsWidget::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (m_menu != nullptr) {
#ifndef RUNNING_UNIT_TESTS
    m_menu->popup(mapToGlobal(event->pos()));  // or QCursor::pos()
#endif
    event->accept();
    return;
  }
  QWidget::contextMenuEvent(event);
}

void RatingStarsWidget::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  for (int i = 0; i < starCount(); ++i) {
    QRect rect = starRect(i);
    QColor color = starColor(i);
    painter.save();
    painter.translate(rect.center());
    double scale = qMin(rect.width(), rect.height()) / 2.0;
    painter.scale(scale, scale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawPath(m_starPath);
    painter.restore();
  }
}

QRect RatingStarsWidget::starRect(int index) const {
  const int starWidth = width() / starCount();
  const int y = 0;
  const int x = index * starWidth;
  const int margin = 0;
  return QRect(x + margin, y + margin, starWidth - 2*margin, height() - 2*margin);
}

QColor RatingStarsWidget::starColor(int index) const {
  constexpr QColor DARK_ORANGE{255, 130, 0};    // 深橙色（用于评分减少时的预览）
  constexpr QColor ORANGE{255, 165, 0};         // 标准橙色（已评分区域）
  constexpr QColor LIGHT_ORANGE{255, 200, 80};  // 亮橙色（评分增加时的预览）
  const int minOne = std::min(m_hoverRating, m_currentRating);
  const int maxOne = std::max(m_hoverRating, m_currentRating);
  if (m_hoverRating >= 0) {
    if (index < minOne) {
      return ORANGE;
    } else if (index < maxOne) {
      return m_hoverRating > m_currentRating ? DARK_ORANGE : LIGHT_ORANGE;
    } else {
      return Qt::gray;
    }
  } else {
    if (index < m_currentRating) {
      return ORANGE;
    } else {
      return Qt::gray;
    }
  }
}

int RatingStarsWidget::GetHoverRate(int pos) const {
  const int starWidth = width() / starCount();
  int x = pos;
  return qBound(0, x / starWidth + 1, starCount());
}

void RatingStarsWidget::mouseMoveEvent(QMouseEvent *event) {
  const int newHover = GetHoverRate(event->pos().x());
  if (newHover != m_hoverRating) {
    m_hoverRating = newHover;
    QToolTip::showText(event->globalPos(), QString::number(newHover), this);
    update();
  }
}

void RatingStarsWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    int newRating = GetHoverRate(event->pos().x());
    if (m_currentRating != newRating) {
      m_currentRating = newRating;
      update();
      emit RateActions::GetInst(RateActions::RateRequestFrom::FILE_XPLORER).RateMovieReq(m_currentRating);
    }
  }
}

void RatingStarsWidget::leaveEvent(QEvent *event) {
  if (m_hoverRating >= 0) {
    m_hoverRating = -1;
    update();
  }
}
