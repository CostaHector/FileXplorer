#ifndef MULTILINEEDITDELEGATE_H
#define MULTILINEEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

class MultiLineEditDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;
  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // MULTILINEEDITDELEGATE_H
