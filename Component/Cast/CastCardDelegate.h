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

  // ---- 编辑态：创建编辑器 ----
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override;
  // ---- 编辑态：模型 → 编辑器 ----
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  // ---- 编辑态：编辑器 → 模型（按自定义 role 写回） ----
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
  // ---- 编辑态：几何 ----
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override;

private:
  QSize m_cardSize{CARD_W, CARD_H};

  static constexpr int CARD_MARGIN  = 4;
  static constexpr int PADDING      = 8;
  static constexpr int LINE_SPACING = 2;
};

#endif  // CASTCARDDELEGATE_H