#include "StyleSheetTreeModel.h"
#include "StyleSheetGetter.h"
#include "Logger.h"
#include <QVariantHash>
#include <QIcon>
#include <QColor>

StyleSheetTreeModel::StyleSheetTreeModel(QObject* parent)
  : QAbstractTreeModelPub<StyleTreeNode>{parent} {
  std::unique_ptr<StyleTreeNode> pRoot{StyleTreeNode::NewTreeNodeRoot()};
  pRoot->setName("StyleSheet");
  initFontRelated(pRoot);
  initColorRelated(pRoot, Style::StyleSheetE::STYLESHEET_LIGHT);
  initColorRelated(pRoot, Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG);
  setDatas(std::move(pRoot));
}

bool StyleSheetTreeModel::initFontRelated(std::unique_ptr<StyleTreeNode>& pRoot) const {
  if (!pRoot) {
    return false;
  }
  using namespace FontCfg;
  auto* pFont = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
  auto* pFontFamily = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Family"}));
  auto* pFontSize = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Size"}));
  auto* pFontWeight = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Weight"}));
  auto* pFontStyle = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Style"}));
  pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontFamilyDef, GetFontFamily(), StyleItemData::FONT_FAMILY}));

  pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontSizeDef, GetFontSize(), StyleItemData::NUMBER}));
  pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"QTabBar", mFontSizeDef, GetFontSizeTab(), StyleItemData::NUMBER}));

  pFontWeight->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontWeightDef, GetFontWeight(), StyleItemData::FONT_WEIGHT}));

  pFontStyle->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontStyleDef, GetFontStyle(), StyleItemData::FONT_STYLE}));
  return true;
}

bool StyleSheetTreeModel::initColorRelated(std::unique_ptr<StyleTreeNode>& pRoot, Style::StyleSheetE styleE) const {
  if (!pRoot) {
    return false;
  }
  using namespace ColorCfg;
  using namespace Style;
  const QString styleName{styleE == StyleSheetE::STYLESHEET_LIGHT ? "LightColor" : "DarkColor"};
  auto* pColor = pRoot->appendRow(StyleTreeNode::create(StyleItemData{styleName}));
  auto* pColorBg = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Background"}));
  auto* pColorBgGeneral = pColorBg->appendRow(StyleTreeNode::create(
      StyleItemData{"General", ColorBackgroundGeneralDef(styleE), GetColorBackgroundGeneral(styleE), StyleItemData::COLOR}));
  auto* pColorBgAlternateRow = pColorBg->appendRow(StyleTreeNode::create(
      StyleItemData{"AlternateRow", ColorBackgroundAlternateRowDef(styleE), GetColorBackgroundAlternateRow(styleE), StyleItemData::COLOR}));
  auto* pColorBgHover = pColorBg->appendRow(StyleTreeNode::create(
      StyleItemData{"Hover", ColorBackgroundHoverDef(styleE), GetColorBackgroundHover(styleE), StyleItemData::COLOR}));
  auto* pColorBgSelectedActive = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"SelectedActive",
                                                                                         ColorBackgroundSelectedActiveDef(styleE),
                                                                                         GetColorBackgroundSelectedActive(styleE),
                                                                                         StyleItemData::COLOR}));
  auto* pColorBgSelectedInactive = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"SelectedInActive",
                                                                                           ColorBackgroundSelectedInactiveDef(styleE),
                                                                                           GetColorBackgroundSelectedInactive(styleE),
                                                                                           StyleItemData::COLOR}));
  auto* pColorBgMenu = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"Menu", ColorBackgroundMenuDef(styleE), GetColorBackgroundMenu(styleE), StyleItemData::COLOR}));
  auto* pColorBgMenuChecked = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"MenuChecked", ColorBackgroundMenuCheckedDef(styleE), GetColorBackgroundMenuChecked(styleE), StyleItemData::COLOR}));
  auto* pColorBgMenuSelected = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"MenuSelected", ColorBackgroundMenuSelectedDef(styleE), GetColorBackgroundMenuSelected(styleE), StyleItemData::COLOR}));


  auto* pColorGridLine = pColor->appendRow(
      StyleTreeNode::create(StyleItemData{"Gridline", ColorGridLineDef(styleE), GetColorGridLine(styleE), StyleItemData::COLOR}));

  auto* pColorBorder = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Border"}));
  auto* pColorBorderGeneral = pColorBorder->appendRow(
      StyleTreeNode::create(StyleItemData{"General", ColorBorderGeneralDef(styleE), GetColorBorderGeneral(styleE), StyleItemData::COLOR}));
  auto* pColorBorderMenuRight = pColorBorder->appendRow(StyleTreeNode::create(
      StyleItemData{"MenuRight", ColorBorderMenuRightDef(styleE), GetColorBorderMenuRight(styleE), StyleItemData::COLOR}));

  auto* pColorForground = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Forground"}));
  auto* pColorForgroundGeneral = pColorForground->appendRow(StyleTreeNode::create(
      StyleItemData{"General", ColorForgroundGeneralDef(styleE), GetColorForgroundGeneral(styleE), StyleItemData::COLOR}));

  return true;
}

QVariant StyleSheetTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  StyleTreeNode* node = static_cast<StyleTreeNode*>(index.internalPointer());
  if (!node) {
    return {};
  }
  const StyleItemData& item = node->value();
  const int column = index.column();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (column) {
      case StyleItemData::DEF_NAME_TEXT_ROLE - StyleItemData::DEF_BEGIN_ROLE:
        return item.name;
      case StyleItemData::DEFAULT_DATA_ROLE - StyleItemData::DEF_BEGIN_ROLE:
        return item.defValue;
      case StyleItemData::CURRENT_DATA_ROLE - StyleItemData::DEF_BEGIN_ROLE:
        return item.curValue;
      case StyleItemData::EDITABLE_COLUMN:
        return item.modifiedToValue;
      default:
        return {};
    }
  } else if (role == StyleItemData::DATA_TYPE_ROLE) {
    return item.dataType;
  } else if (role == Qt::DecorationRole) {
    static QIcon failedIcon{":img/NOT_SAVED"};
    if (column == StyleItemData::EDITABLE_COLUMN && mEditFailedCells.contains(index)) {
      return failedIcon;
    }
    if (item.dataType == StyleItemData::DataTypeE::COLOR) {
      QString colorStr;
      switch (column) {
        case StyleItemData::DEFAULT_DATA_ROLE - StyleItemData::DEF_BEGIN_ROLE:
          colorStr = item.defValue.toString();
          break;
        case StyleItemData::CURRENT_DATA_ROLE - StyleItemData::DEF_BEGIN_ROLE:
          colorStr = item.curValue.toString();
          break;
        case StyleItemData::EDITABLE_COLUMN:
          colorStr = item.modifiedToValue.toString();
          break;
        default:
          return {};
      }
      if (colorStr.isEmpty()) {
        return {};
      }
      QColor color(colorStr);
      if (!color.isValid()) {
        return {};
      }
      QPixmap pixmap(16, 16);
      pixmap.fill(color);
      return pixmap;
    }
  }
  return {};
}

bool StyleSheetTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }
  if (role != Qt::EditRole) {
    return false;
  }
  if (!flags(index).testFlag(Qt::ItemIsEditable)) {
    return false;
  }
  StyleTreeNode* node = static_cast<StyleTreeNode*>(index.internalPointer());
  if (node == nullptr) {
    return false;
  }
  StyleItemData& item = node->value();
  bool bNewValueValid{true};
  const bool bChanged{item.setModifiedToValue(value, bNewValueValid)};
  editCell(index, bNewValueValid);
  if (!bChanged) {
    return false;
  }
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  if (m_bInstantApply) {
    QString cfgKey = node->GetConfigKey();
    emit requestApplyChanges(cfgKey, value);
  }
  return true;
}

void StyleSheetTreeModel::editCell(const QModelIndex& index, bool bSucceed) {
  if (bSucceed) {
    editCellSucceed(index);
  } else {
    editCellFailed(index);
  }
}

void StyleSheetTreeModel::editCellFailed(const QModelIndex& failedInd) {
  if (!failedInd.isValid()) {
    return;
  }
  if (mEditFailedCells.contains(failedInd)) {
    return;
  }
  mEditFailedCells.insert(failedInd);
  emit dataChanged(failedInd, failedInd, {Qt::DecorationRole});
}

void StyleSheetTreeModel::editCellSucceed(const QModelIndex& okInd) {
  if (!editCellEraseIndex(okInd)) {
    return;
  }
  emit dataChanged(okInd, okInd, {Qt::DecorationRole});
}

bool StyleSheetTreeModel::editCellEraseIndex(const QModelIndex& okInd) {
  if (!okInd.isValid()) {
    return false;
  }
  auto it = mEditFailedCells.find(okInd);
  if (it == mEditFailedCells.end()) {
    return false;
  }
  mEditFailedCells.erase(it);
  return true;
}

Qt::ItemFlags StyleSheetTreeModel::flags(const QModelIndex& index) const {
  // root: drop only
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }
  // group: drag and drop
  if (isGroup(index)) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  if (index.column() == StyleItemData::EDITABLE_COLUMN) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  }
  // nongroup, drag only
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int StyleSheetTreeModel::ClearNewValues(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const int N = rowCount();
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    const int row = ind.row();
    if (row < 0 || row >= N) {
      LOG_W("row[%d] out of range", row);
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", row);
      continue;
    }
    StyleItemData& item = node->value();
    if (!item.invalidateNewValue()) {
      continue; // group: no need clear
    }

    const QModelIndex editInd{siblingAtColumn(ind, StyleItemData::EDITABLE_COLUMN)};
    editCellEraseIndex(editInd);
    emit dataChanged(editInd, editInd, affectedRoles);
    ++affectedRows;
  }
  LOG_D("%d in %d new value field get updated", affectedRows, indexes.size());
  return affectedRows;
}

int StyleSheetTreeModel::recoverNewValuesToBackup(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const int N = rowCount();
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    const int row = ind.row();
    if (row < 0 || row >= N) {
      LOG_W("row[%d] out of range", row);
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", row);
      continue;
    }
    StyleItemData& item = node->value();
    if (!item.recoverToBackup()) {
      continue; // group: no need clear
    }
    const QModelIndex editInd{siblingAtColumn(ind, StyleItemData::EDITABLE_COLUMN)};
    editCellEraseIndex(editInd);
    emit dataChanged(editInd, editInd, affectedRoles);
    ++affectedRows;
  }
  LOG_D("%d in %d new value field get reset to its backup", affectedRows, indexes.size());
  return affectedRows;
}

QVariantHash StyleSheetTreeModel::ApplyNewValues(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return {};
  }

  const int N{columnCount()};
  QVariantHash key2Cfg;
  for (const QModelIndex& ind : indexes) {
    const int row = ind.row();
    if (row < 0 || row >= N) {
      LOG_W("row[%d] out of range", row);
      continue;
    }
    const StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", row);
      continue;
    }
    if (node->isGroup()) {
      continue;
    }
    const StyleItemData& item = node->value();
    if (!item.isModifiedToValueValid()) {
      continue;
    }
    QString cfgKey = node->GetConfigKey();
    key2Cfg[cfgKey] = item.modifiedToValue;
  }

  const int affectedRows{key2Cfg.size()};
  LOG_D("%d in %d duration field get updated", affectedRows, indexes.size());
  return key2Cfg;
}
