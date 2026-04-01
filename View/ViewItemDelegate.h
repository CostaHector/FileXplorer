#ifndef VIEWITEMDELEGATE_H
#define VIEWITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "SizeTool.h"

class ViewItemDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return {QStyledItemDelegate::sizeHint(option, index).width(), SizeTool::TABLE_DEFAULT_ROW_SECTION_SIZE};
  }
};

#endif  // VIEWITEMDELEGATE_H
