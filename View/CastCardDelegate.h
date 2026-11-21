#ifndef CASTCARDDELEGATE_H
#define CASTCARDDELEGATE_H

#include <QStyledItemDelegate>

class CastCardDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  // 默认卡片槽位(竖版球员卡比例 7:10)
  static constexpr int CARD_W = 175;
  static constexpr int CARD_H = 250;

  void setCardSize(const QSize& newSize) { m_cardSize = newSize; }
  QSize cardSize() const { return m_cardSize; }

  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override;
  void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;

private:
  QSize m_cardSize{CARD_W, CARD_H};
};

#endif  // CASTCARDDELEGATE_H