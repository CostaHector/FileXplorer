#ifndef VIEWITEMDELEGATE_H
#define VIEWITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "SizeTool.h"

class ViewItemDelegate : public QStyledItemDelegate {
 public:
  explicit ViewItemDelegate(QObject *parent = nullptr);

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return {QStyledItemDelegate::sizeHint(option, index).width(), m_rowHeight};
  }

  bool setRowHeight(int newRowHeight);

  int rowHeight() const { return m_rowHeight; }

 private:
  int m_rowHeight = SizeTool::TABLE_DEFAULT_ROW_SECTION_SIZE;
};

#endif  // VIEWITEMDELEGATE_H
