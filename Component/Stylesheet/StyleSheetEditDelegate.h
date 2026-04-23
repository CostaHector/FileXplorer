#ifndef STYLESHEETEDITDELEGATE_H
#define STYLESHEETEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>
#include <QMap>

class StyleSheetEditDelegate : public QStyledItemDelegate {
public:
  explicit StyleSheetEditDelegate(QObject *parent = nullptr);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  QWidget* createLineEditorWithColorDialog(QWidget* editWidget) const;
  QWidget* createLineEditorWithFileDialog(QWidget* editWidget) const;

  QStringList mFontFamilyItems;
  QMap<QString, QFont::Weight> mFontWeightItems;
  QMap<QString, QFont::Style> mFontStyleItems;
};
#endif // STYLESHEETEDITDELEGATE_H
