#include "MultiLineEditDelegate.h"
#include <QPlainTextEdit>

MultiLineEditDelegate::MultiLineEditDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *MultiLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
  QPlainTextEdit *editor = new QPlainTextEdit(parent);
  return editor;
}

void MultiLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QPlainTextEdit *mEditor = qobject_cast<QPlainTextEdit *>(editor);
  mEditor->setPlainText(index.data().toString());
}

void MultiLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QPlainTextEdit *mEditor = qobject_cast<QPlainTextEdit *>(editor);
  model->setData(index, mEditor->toPlainText());
}
