#ifndef STYLESHEETEDITDELEGATE_H
#define STYLESHEETEDITDELEGATE_H

#include <QStyledItemDelegate>

class StyleSheetEditDelegate : public QStyledItemDelegate {
public:
  explicit StyleSheetEditDelegate(int dataTypeRole = Qt::ItemDataRole::UserRole + 1, int editableColumn = 0, QObject *parent = nullptr);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
  void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
  const int mDataTypeRole, mEditableColumn;
};
#endif // STYLESHEETEDITDELEGATE_H
