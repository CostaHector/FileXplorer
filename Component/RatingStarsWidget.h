#ifndef RATINGSTARSWIDGET_H
#define RATINGSTARSWIDGET_H

#include <QWidget>
#include <QPainterPath>
class QMenu;

class RatingStarsWidget : public QWidget {
public:
  static RatingStarsWidget* GInstance(RatingStarsWidget* pInitValue=nullptr, bool bReset=false);
  explicit RatingStarsWidget(QWidget *parent = nullptr);

  void freshRating(int value);
  int rating() const { return m_currentRating; }

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;

private:
  QPainterPath GetStarPath() const;
  int GetHoverRate(int pos) const;
  int clampRate(int rate) const {
    return rate < 0 ? 0 : (rate > starCount() ? starCount() : rate);
  }
  constexpr int starCount() const { return 10; }
  int m_currentRating = 0;
  int m_hoverRating = -1;                 // < 0 means no hovering

  QRect starRect(int index) const;
  QColor starColor(int index) const;
  const QPainterPath m_starPath;

  QMenu* m_menu{nullptr};
};

#endif // RATINGSTARSWIDGET_H
