#ifndef ACCOUNTCARDDELEGATE_H
#define ACCOUNTCARDDELEGATE_H

#include <QStyledItemDelegate>

class AccountCardDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  // ---- 常态尺寸：7 行 × 行高 + 间距 + padding ----
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override;

  // ---- 常态绘制 ----
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  // ---- 编辑态：创建编辑器 ----
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override;

  // ---- 编辑态：模型 → 编辑器 ----
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  // ---- 编辑态：编辑器 → 模型（按自定义 role 写回） ----
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

  // ---- 编辑态：几何 ----
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override;

private:
  static constexpr int CARD_MARGIN  = 4;
  static constexpr int PADDING      = 8;
  static constexpr int LINE_SPACING = 2;
};

#endif // ACCOUNTCARDDELEGATE_H
