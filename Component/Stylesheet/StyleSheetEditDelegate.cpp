#include "StyleSheetEditDelegate.h"
#include "Logger.h"
#include "GeneralDataType.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "ComboBoxGeneral.h"
#include "LineEditGeneral.h"
#include "PlainTextEditGeneral.h"
#include "ColorTools.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QPainter>

StyleSheetEditDelegate::StyleSheetEditDelegate(int dataTypeRole, int editableColumn, QObject *parent)
  : QStyledItemDelegate(parent)
  , mDataTypeRole{dataTypeRole}
  , mEditableColumn{editableColumn} {}

QWidget *StyleSheetEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  if (index.column() != mEditableColumn) {
    return nullptr;
  }
  const int dataType = index.data(mDataTypeRole).toInt();

  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    return ComboBoxGeneral::create(dataType, parent);
  }
  if (GeneralDataType::isLineEditWithActionNeededInEditor(dataType)) {
    return LineEditGeneral::create(dataType, parent);
  }
  if (GeneralDataType::isPlainTextEditNeededInEditor(dataType)) {
    return PlainTextEditGeneral::create(dataType, parent);
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}

void StyleSheetEditDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
  QStyledItemDelegate::initStyleOption(option, index);
  if (!index.isValid()) {
    return;
  }
  if (index.column() != mEditableColumn) {
    return;
  }
  const int dataType = index.data(mDataTypeRole).toInt();
  const QVariant displayRoleData = index.data(Qt::DisplayRole);
  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    option->text = ComboBoxGeneral::displayText(dataType, displayRoleData);
    return;
  }
}

void StyleSheetEditDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  CHECK_NULLPTR_RETURN_VOID(painter);
  // 1. plot basic image and text
  QStyledItemDelegate::paint(painter, option, index);

  if (index.column() != mEditableColumn) {
    return;
  }
  const int dataType = index.data(mDataTypeRole).toInt();
  if (dataType == GeneralDataType::Type::COMMA_SEPERATED_STR_LIST) {
    QRect currentRateTextRect = option.rect;
    currentRateTextRect.setHeight(20);
    currentRateTextRect.setWidth(20);
    painter->fillRect(currentRateTextRect, ColorTools::GRAY_AND_HALF_TRANS);
    painter->setPen(Qt::white);
    const int cnt = PlainTextEditGeneral::linesCount(dataType, index.model()->data(index, Qt::DisplayRole));
    painter->drawText(currentRateTextRect, Qt::AlignCenter, QString::number(cnt));
  }
}

// 从模型中获取指定索引（index）的数据，并将其设置到编辑器中，以便用户可以看到当前值并进行编辑。
void StyleSheetEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (index.column() != mEditableColumn) {
    return;
  }
  const int dataType = index.data(mDataTypeRole).toInt();
  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    ComboBoxGeneral *comboBox = dynamic_cast<ComboBoxGeneral *>(editor);
    if (!comboBox) {
      LOG_E("Editor is not a GeneralComboBox for dataType[%d]", dataType);
      return;
    }
    QVariant editRoleData = index.model()->data(index, Qt::EditRole);
    comboBox->updateCurrentTextFromEditRole(editRoleData);
    return;
  }
  if (GeneralDataType::isPlainTextEditNeededInEditor(dataType)) {
    PlainTextEditGeneral *plainTextEdit = dynamic_cast<PlainTextEditGeneral *>(editor);
    if (!plainTextEdit) {
      LOG_E("Editor is not a QPlainTextEdit for dataType[%d]", dataType);
      return;
    }
    QVariant editRoleData = index.model()->data(index, Qt::EditRole);
    plainTextEdit->updateCurrentTextFromEditRole(editRoleData);
    return;
  }
  QStyledItemDelegate::setEditorData(editor, index);
}

void StyleSheetEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (index.column() != mEditableColumn) {
    return;
  }
  const int dataType = index.data(mDataTypeRole).toInt();
  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    const ComboBoxGeneral *comboBox = dynamic_cast<const ComboBoxGeneral *>(editor);
    if (!comboBox) {
      LOG_E("Editor is not a GeneralComboBox for dataType[%d]", dataType);
      return;
    }
    QVariant editRoleValue = comboBox->getSetDataEditRoleValue();
    bool setResult = model->setData(index, editRoleValue, Qt::EditRole);
    LOG_OE_P(setResult, "setModelData", "dataType[%d], index[(%d, %d)]", dataType, index.row(), index.column());
    return;
  }
  if (GeneralDataType::isLineEditWithActionNeededInEditor(dataType)) {
    LineEditGeneral *lineEdit = dynamic_cast<LineEditGeneral *>(editor);
    if (!lineEdit) {
      LOG_E("Editor is not a LineEditGeneral for dataType[%d]", dataType);
      return;
    }
    const QVariant editRoleValue = lineEdit->getSetDataEditRoleValue();
    bool setResult = model->setData(index, editRoleValue, Qt::EditRole);
    LOG_OE_P(setResult, "setModelData", "dataType[%d], index[(%d, %d)]", dataType, index.row(), index.column());
    return;
  }
  if (GeneralDataType::isPlainTextEditNeededInEditor(dataType)) {
    PlainTextEditGeneral *plainTextEdit = dynamic_cast<PlainTextEditGeneral *>(editor);
    if (!plainTextEdit) {
      LOG_E("Editor is not a QPlainTextEdit for dataType[%d]", dataType);
      return;
    }
    const QString editRoleValue = plainTextEdit->getSetDataEditRoleValue();
    model->setData(index, editRoleValue, Qt::EditRole); // no need checking here
    return;
  }
  QStyledItemDelegate::setModelData(editor, model, index);
}

void StyleSheetEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  CHECK_NULLPTR_RETURN_VOID(editor);
  editor->setGeometry(option.rect);
}
