#include "StyleSheetEditDelegate.h"
#include "StyleItemData.h"
#include "Logger.h"
#include "PublicMacro.h"

StyleSheetEditDelegate::StyleSheetEditDelegate(QObject *parent)
  : QStyledItemDelegate(parent) {
  mFontFamilyItems = decltype(mFontFamilyItems){
#ifdef _WIN32
      // Windows平台字体
      "Microsoft YaHei",    // 微软雅黑
      "SimSun",             // 宋体
      "NSimSun",            // 新宋体
      "Microsoft JhengHei", // 微软正黑体
      "Arial",              // 英文无衬线字体
      "Times New Roman",    // 英文衬线字体
      "Tahoma",             // Windows系统UI字体
      "Segoe UI",           // Windows现代UI字体
      "Calibri",            // Office默认字体
      "Consolas",           // 等宽字体
      "Courier New",        // 等宽字体
      "Verdana",            // 屏幕显示优化字体
      "Georgia",            // 适合屏幕阅读的衬线字体
      "Trebuchet MS",       // Web安全字体
      "Comic Sans MS"       // 手写风格字体
#else
      // Linux平台字体
      "Noto Sans",        // Google跨平台字体
      "Noto Sans CJK SC", // Noto Sans中文字体
      "DejaVu Sans",      // 开源无衬线字体
      "DejaVu Serif",     // 开源衬线字体
      "Liberation Sans",  // 替换Arial的开源字体
      "Liberation Serif", // 替换Times New Roman的开源字体
      "Ubuntu",           // Ubuntu系统默认字体
      "FreeSans",         // 开源无衬线字体
      "Droid Sans",       // Android系统字体
      "Arial",            // 英文无衬线字体
      "Times New Roman",  // 英文衬线字体
      "Tahoma",           // 屏幕显示字体
      "Verdana",          // 屏幕显示优化字体
      "Courier New",      // 等宽字体
      "Monospace"         // 通用等宽字体
#endif
  };
  mFontWeightItems = decltype(mFontWeightItems){
      {"Thin", QFont::Weight::Thin},
      {"ExtraLight", QFont::Weight::ExtraLight},
      {"Light", QFont::Weight::Light},
      {"Normal", QFont::Weight::Normal},
      {"Medium", QFont::Weight::Medium},
      {"DemiBold", QFont::Weight::DemiBold},
      {"Bold", QFont::Weight::Bold},
      {"ExtraBold", QFont::Weight::ExtraBold},
      {"Black", QFont::Weight::Black},
  };
  mFontStyleItems = decltype(mFontStyleItems){
      {"StyleNormal", QFont::Style::StyleNormal},
      {"StyleItalic", QFont::Style::StyleItalic},
      {"StyleOblique", QFont::Style::StyleOblique},
  };
}

QWidget *StyleSheetEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  const int dataType = index.data(StyleItemData::Role::DATA_TYPE_ROLE).toInt();
  if (dataType == StyleItemData::DataTypeE::NUMBER || dataType == StyleItemData::DataTypeE::COLOR || index.column() != StyleItemData::EDITABLE_COLUMN) {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  QComboBox *editor = new QComboBox(parent);
  editor->setEditable(false);
  switch (dataType) {
    case StyleItemData::DataTypeE::FONT_FAMILY:
      editor->addItems(mFontFamilyItems);
      break;
    case StyleItemData::DataTypeE::FONT_WEIGHT:
      editor->addItems(mFontWeightItems.keys());
      break;
    case StyleItemData::DataTypeE::FONT_STYLE:
      editor->addItems(mFontStyleItems.keys());
      break;
    default:
      LOG_E("dataType invalid[%d]", dataType);
      editor->setEditable(true);
      break;
  }
  return editor;
}

void StyleSheetEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  const int dataType = index.data(StyleItemData::Role::DATA_TYPE_ROLE).toInt();
  if (dataType == StyleItemData::DataTypeE::NUMBER || dataType == StyleItemData::DataTypeE::COLOR || index.column() != StyleItemData::EDITABLE_COLUMN) {
    QStyledItemDelegate::setEditorData(editor, index);
    return;
  }
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  QString rawText = index.model()->data(index, Qt::EditRole).toString();
  switch (dataType) {
    case StyleItemData::DataTypeE::FONT_FAMILY:
      if (!mFontFamilyItems.contains(rawText)) {
        LOG_W("font-family[%s] invalid", qPrintable(rawText));
        rawText = "";
      }
      break;
    case StyleItemData::DataTypeE::FONT_WEIGHT:
      if (!mFontWeightItems.contains(rawText)) {
        LOG_W("font-weight[%s] invalid", qPrintable(rawText));
        rawText = "";
      }
      break;
    case StyleItemData::DataTypeE::FONT_STYLE:
      if (!mFontStyleItems.contains(rawText)) {
        LOG_W("font-style[%s] invalid", qPrintable(rawText));
        rawText = "";
      }
      break;
    case StyleItemData::DataTypeE::COLOR:
      if (!QColor(rawText).isValid()) {
        LOG_W("color[%s] invalid", qPrintable(rawText));
        rawText = "";
      }
      break;
    default:
      LOG_E("dataType invalid[%d]", dataType);
      return;
  }
  comboBox->setCurrentText(rawText);
}

void StyleSheetEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  const int dataType = index.data(StyleItemData::Role::DATA_TYPE_ROLE).toInt();
  if (dataType == StyleItemData::DataTypeE::NUMBER || dataType == StyleItemData::DataTypeE::COLOR || index.column() != StyleItemData::EDITABLE_COLUMN) {
    QStyledItemDelegate::setModelData(editor, model, index);
    return;
  }
  const QComboBox *comboBox = static_cast<QComboBox *>(editor);
  CHECK_NULLPTR_RETURN_VOID(comboBox);

  QString rawText{comboBox->currentText()};
  QVariant newValueData;
  switch (dataType) {
    case StyleItemData::DataTypeE::FONT_FAMILY:
      if (mFontFamilyItems.contains(rawText)) {
        newValueData = rawText;
      } else {
        LOG_E("font-family[%s] invalid", qPrintable(rawText));
        newValueData = "";
      }
      break;
    case StyleItemData::DataTypeE::FONT_WEIGHT: {
      newValueData = mFontWeightItems.value(rawText, QFont::Weight::Normal);
      break;
    }
    case StyleItemData::DataTypeE::FONT_STYLE: {
      newValueData = mFontStyleItems.value(rawText, QFont::Style::StyleNormal);
      break;
    }
    case StyleItemData::DataTypeE::COLOR: {
      if (QColor(rawText).isValid()) {
        newValueData = rawText;
      } else {
        LOG_W("color[%s] invalid", qPrintable(rawText));
        newValueData = "";
      }
      break;
    }
    default:
      LOG_E("dataType invalid[%d]", dataType);
      return;
  }
  model->setData(index, newValueData, Qt::EditRole);
}

void StyleSheetEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}
