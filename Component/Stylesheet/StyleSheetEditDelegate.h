#ifndef STYLESHEETEDITDELEGATE_H
#define STYLESHEETEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>
#include <QMap>

class StyleSheetEditDelegate : public QStyledItemDelegate {
public:
  explicit StyleSheetEditDelegate(int dataTypeRole = Qt::ItemDataRole::UserRole, int editableColumn = 0, QObject *parent = nullptr);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  QWidget *createLineEditorWithColorDialog(QWidget *editWidget) const;
  QWidget *createLineEditorWithFileDialog(QWidget *editWidget) const;
  QWidget *createLineEditorWithFolderDialog(QWidget *editWidget) const;

  bool setComboBoxModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, int dataType) const;

  const int mDataTypeRole, mEditableColumn;
};
#endif // STYLESHEETEDITDELEGATE_H
