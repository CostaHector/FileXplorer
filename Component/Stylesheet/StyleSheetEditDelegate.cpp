#include "StyleSheetEditDelegate.h"
#include "Logger.h"
#include "GeneralDataType.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "GeneralComboBox.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>

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
    return GeneralComboBox::create(dataType, parent);
  }

  switch (dataType) {
    case GeneralDataType::Type::COLOR: {
      auto *editWidget = QStyledItemDelegate::createEditor(parent, option, index);
      return createLineEditorWithColorDialog(editWidget);
    }
    case GeneralDataType::Type::FILE_PATH:
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL: {
      auto *editWidget = QStyledItemDelegate::createEditor(parent, option, index);
      return createLineEditorWithFileDialog(editWidget);
    }
    case GeneralDataType::Type::FOLDER_PATH: {
      auto *editWidget = QStyledItemDelegate::createEditor(parent, option, index);
      return createLineEditorWithFolderDialog(editWidget);
    }
    case GeneralDataType::Type::MULTI_LINE_STR: {
      return new QPlainTextEdit{parent};
    }
    default:
      return QStyledItemDelegate::createEditor(parent, option, index);
  }
}

QWidget *StyleSheetEditDelegate::createLineEditorWithColorDialog(QWidget *editWidget) const {
  QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editWidget);
  if (lineEditor == nullptr) {
    LOG_W("editWidget is not QLineEdit, cannot add action");
    return editWidget;
  }
  QAction *colorAction = lineEditor->addAction(QIcon(":/styles/COLOR_SELECT"), QLineEdit::LeadingPosition);
  connect(colorAction, &QAction::triggered, lineEditor, [lineEditor]() {
    QColor newColor = QColorDialog::getColor(Qt::GlobalColor::white, lineEditor, "Select color");
    if (!newColor.isValid()) {
      LOG_INFO_NP("Skip", "User cancel select a color");
      return;
    }
    lineEditor->setText(newColor.name(QColor::HexArgb));
  });
  return lineEditor;
}

QWidget *StyleSheetEditDelegate::createLineEditorWithFileDialog(QWidget *editWidget) const {
  QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editWidget);
  if (lineEditor == nullptr) {
    LOG_W("editWidget is not QLineEdit, cannot add action");
    return editWidget;
  }
  QAction *imgfileSelectAction = lineEditor->addAction(QIcon{":img/FILE"}, QLineEdit::LeadingPosition);
  connect(imgfileSelectAction, &QAction::triggered, lineEditor, [lineEditor]() {
    static const QString imgFilters = []() -> QString {
      QString filtersStr;
      filtersStr.reserve(100);
      filtersStr += "Image Files (";
      filtersStr += TYPE_FILTER::IMAGE_TYPE_SET.join(' ');
      filtersStr += ")";
      return filtersStr;
    }();
    const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog};
    QString imageFile = QFileDialog::getOpenFileName(lineEditor,
                                                     "Select an image file",
                                                     SystemPath::HOME_PATH() + "/Pictures",
                                                     imgFilters, //
                                                     nullptr,
                                                     parmOptions);
    if (imageFile.isEmpty()) {
      LOG_INFO_NP("Skip", "User cancel select an image file");
      return;
    }
    lineEditor->setText(PathTool::normPath(imageFile));
  });
  return lineEditor;
}

QWidget *StyleSheetEditDelegate::createLineEditorWithFolderDialog(QWidget *editWidget) const {
  QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editWidget);
  if (lineEditor == nullptr) {
    LOG_W("editWidget is not QLineEdit, cannot add action");
    return editWidget;
  }
  QAction *folderSelectAction = lineEditor->addAction(QIcon{":img/FOLDER"}, QLineEdit::LeadingPosition);
  connect(folderSelectAction, &QAction::triggered, lineEditor, [lineEditor]() {
    const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog | QFileDialog::Option::ShowDirsOnly};
    QString folderPath = QFileDialog::getExistingDirectory(lineEditor, "Select an folder", SystemPath::HOME_PATH(), parmOptions);
    if (folderPath.isEmpty()) {
      LOG_INFO_NP("Skip", "User cancel select an folder");
      return;
    }
    lineEditor->setText(PathTool::normPath(folderPath));
  });
  return lineEditor;
}

// 从模型中获取指定索引（index）的数据，并将其设置到编辑器中，以便用户可以看到当前值并进行编辑。
void StyleSheetEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (index.column() != mEditableColumn) {
    return;
  }
  const int dataType = index.data(mDataTypeRole).toInt();
  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    GeneralComboBox *comboBox = dynamic_cast<GeneralComboBox *>(editor);
    if (!comboBox) {
      LOG_E("Editor is not a GeneralComboBox for dataType[%d]", dataType);
      return;
    }
    QVariant editRoleData = index.model()->data(index, Qt::EditRole);
    comboBox->updateCurrentDisplayString(editRoleData);
    return;
  }
  QStyledItemDelegate::setEditorData(editor, index);
}

void StyleSheetEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  if (index.column() != mEditableColumn) {
    return;
  }
  bool setResult{true};
  const int dataType = index.data(mDataTypeRole).toInt();
  if (GeneralDataType::isComboBoxNeededInEditor(dataType)) {
    setResult = setComboBoxModelData(editor, model, index, dataType);
  } else if (GeneralDataType::isPlainTextEditNeededInEditor(dataType)) {
    QPlainTextEdit *plainTextEdit = qobject_cast<QPlainTextEdit *>(editor);
    if (!plainTextEdit) {
      LOG_E("Editor is not a QPlainTextEdit for dataType[%d]", dataType);
      return;
    }
    QString rawText = plainTextEdit->toPlainText();
    setResult = model->setData(index, rawText, Qt::EditRole);
  } else if (GeneralDataType::isPathRelatedType(dataType)) {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (!lineEdit) {
      LOG_E("Editor is not a QLineEdit for dataType[%d]", dataType);
      return;
    }
    const QString stdPath = PathTool::normPath(lineEdit->text());
    setResult = model->setData(index, stdPath, Qt::EditRole);
  } else {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
  if (!setResult) {
    LOG_WARN_P("setModelData Failed", "dataType[%d], index[(%d, %d)]", dataType, index.row(), index.column());
  }
}

void StyleSheetEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  CHECK_NULLPTR_RETURN_VOID(editor);
  editor->setGeometry(option.rect);
}

bool StyleSheetEditDelegate::setComboBoxModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, int dataType) const {
  const GeneralComboBox *comboBox = dynamic_cast<const GeneralComboBox *>(editor);
  if (!comboBox) {
    LOG_E("Editor is not a GeneralComboBox for dataType[%d]", dataType);
    return false;
  }
  QVariant value = comboBox->getSetDataVariant();
  return model->setData(index, value, Qt::EditRole);
}
