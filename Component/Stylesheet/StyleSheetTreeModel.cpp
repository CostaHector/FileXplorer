#include "StyleSheetTreeModel.h"
#include "StyleSheetGetter.h"
#include "Logger.h"
#include <QVariantHash>
#include <QIcon>
#include <QColor>

StyleSheetTreeModel::StyleSheetTreeModel(QObject* parent)
  : QAbstractTreeModelPub<StyleTreeNode>{parent} {
  std::unique_ptr<StyleTreeNode> pRoot{StyleTreeNode::NewTreeNodeRoot("StyleSheet")};
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
  auto* pFontFamilyGeneral = pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontFamilyDef, GetFontFamily(), StyleItemData::FONT_FAMILY}));

  auto* pFontSize = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Size"}));
  auto* pFontSizeGeneral = pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontSizeDef, GetFontSize(), StyleItemData::NUMBER}));
  auto* pFontSizeQTabBar = pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"QTabBar", mFontSizeDef, GetFontSizeTab(), StyleItemData::NUMBER}));

  auto* pFontWeight = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Weight"}));
  auto* pFontWeightGeneral = pFontWeight->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontWeightDef, GetFontWeight(), StyleItemData::FONT_WEIGHT}));

  auto* pFontStyle = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Style"}));
  auto* pFontStyleGeneral = pFontStyle->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontStyleDef, GetFontStyle(), StyleItemData::FONT_STYLE}));
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
  auto* pColorBgGeneral = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"General", ColorBackgroundGeneralDef(styleE), GetColorBackgroundGeneral(styleE), StyleItemData::COLOR}));
  auto* pColorBgAlternateRow = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"AlternateRow", ColorBackgroundAlternateRowDef(styleE), GetColorBackgroundAlternateRow(styleE), StyleItemData::COLOR}));
  auto* pColorBgHover = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"Hover", ColorBackgroundHoverDef(styleE), GetColorBackgroundHover(styleE), StyleItemData::COLOR}));
  auto* pColorBgSelectedActive = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"SelectedActive", ColorBackgroundSelectedActiveDef(styleE), GetColorBackgroundSelectedActive(styleE), StyleItemData::COLOR}));
  auto* pColorBgSelectedInactive = pColorBg->appendRow(
      StyleTreeNode::create(StyleItemData{"SelectedInActive", ColorBackgroundSelectedInactiveDef(styleE), GetColorBackgroundSelectedInactive(styleE), StyleItemData::COLOR}));
  auto* pColorBgMenu = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"Menu"}));
  auto* pColorBgMenuChecked = pColorBgMenu->appendRow(
      StyleTreeNode::create(StyleItemData{"MenuChecked", ColorBackgroundMenuCheckedDef(styleE), GetColorBackgroundMenuChecked(styleE), StyleItemData::COLOR}));
  auto* pColorBgMenuSelected = pColorBgMenu->appendRow(
      StyleTreeNode::create(StyleItemData{"MenuSelected", ColorBackgroundMenuSelectedDef(styleE), GetColorBackgroundMenuSelected(styleE), StyleItemData::COLOR}));

  auto* pColorGridLine = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Gridline", ColorGridLineDef(styleE), GetColorGridLine(styleE), StyleItemData::COLOR}));

  auto* pColorBorder = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Border"}));
  auto* pColorBorderGeneral = pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"General", ColorBorderGeneralDef(styleE), GetColorBorderGeneral(styleE), StyleItemData::COLOR}));
  auto* pColorBorderMenuRight = pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"MenuRight", ColorBorderMenuRightDef(styleE), GetColorBorderMenuRight(styleE), StyleItemData::COLOR}));

  auto* pColorForeground = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Foreground"}));
  auto* pColorForegroundGeneral = pColorForeground->appendRow(
      StyleTreeNode::create(StyleItemData{"General", ColorForegroundGeneralDef(styleE), GetColorForegroundGeneral(styleE), StyleItemData::COLOR}));

  return true;
}

QVariant StyleSheetTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const StyleTreeNode* node = static_cast<StyleTreeNode*>(index.internalPointer());
  if (node == nullptr) {
    return {};
  }
  const StyleItemData& item = node->value();
  const int column = index.column();
  switch (role) {
    case Qt::DisplayRole: {
      switch (column) {
        case StyleItemData::NAME_COLUMN:
          return item.name;
        case StyleItemData::DEF_COLUMN:
          return item.defValue;
        case StyleItemData::CUR_COLUMN:
          return item.curValue;
        case StyleItemData::EDITABLE_COLUMN:
          return item.modifiedToValue;
        default:
          return {};
      }
    }
    case Qt::EditRole: {
      if (column == StyleItemData::EDITABLE_COLUMN) {
        return item.modifiedToValue;
      }
      return {};
    }
    case Qt::DecorationRole: {
      static QIcon failedIcon{":img/NOT_SAVED"};
      if (column == StyleItemData::EDITABLE_COLUMN && mEditFailedCells.contains(index)) {
        return failedIcon;
      }
      if (item.dataType == StyleItemData::DataTypeE::COLOR) {
        QString colorStr;
        switch (column) {
          case StyleItemData::DEF_COLUMN:
            colorStr = item.defValue.toString();
            break;
          case StyleItemData::CUR_COLUMN:
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
      return {};
    }
    case StyleItemData::DATA_TYPE_ROLE:
      return item.dataType;
    default:
      return {};
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
  bool bNewValueAccepted{false};
  const bool bChanged{item.modifyValueTo(value, bNewValueAccepted)};
  editCell(index, bNewValueAccepted);
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

bool StyleSheetTreeModel::editCellFailed(const QModelIndex& failedInd) {
  if (!failedInd.isValid()) {
    return false;
  }
  if (mEditFailedCells.contains(failedInd)) {
    return false;
  }
  mEditFailedCells.insert(failedInd);
  emit dataChanged(failedInd, failedInd, {Qt::DecorationRole});
  return true;
}

bool StyleSheetTreeModel::editCellSucceed(const QModelIndex& okInd) {
  if (!editCellEraseIndex(okInd)) {
    return false;
  }
  emit dataChanged(okInd, okInd, {Qt::DecorationRole});
  return true;
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
  // group: select only
  if (isGroup(index)) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  if (index.column() == StyleItemData::EDITABLE_COLUMN) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  }
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int StyleSheetTreeModel::SetNewColors(const QModelIndexList& indexes, const QString& newColor) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    if (!ind.isValid()) {
      LOG_W("row[%d] out of range", ind.row());
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", ind.row());
      continue;
    }
    StyleItemData& item = node->value();
    if (!item.modifiedColorTo(newColor)) {
      continue;
    }
    const QModelIndex editInd{siblingAtColumn(ind, StyleItemData::EDITABLE_COLUMN)};
    editCellEraseIndex(editInd);
    emit dataChanged(editInd, editInd, affectedRoles);
    ++affectedRows;
  }
  LOG_D("%d in %d new color value field get updated", affectedRows, indexes.size());
  return affectedRows;
}

int StyleSheetTreeModel::ClearNewValues(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    if (!ind.isValid()) {
      LOG_W("row[%d] out of range", ind.row());
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", ind.row());
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

int StyleSheetTreeModel::RecoverNewValuesToDefault(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    if (!ind.isValid()) {
      LOG_W("row[%d] out of range", ind.row());
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", ind.row());
      continue;
    }
    StyleItemData& item = node->value();
    if (!item.recoverToDefault()) {
      continue; // group: no need recover to default
    }
    const QModelIndex editInd{siblingAtColumn(ind, StyleItemData::EDITABLE_COLUMN)};
    editCellEraseIndex(editInd);
    emit dataChanged(editInd, editInd, affectedRoles);
    ++affectedRows;
  }
  LOG_D("%d in %d new value field get reset to its default", affectedRows, indexes.size());
  return affectedRows;
}

int StyleSheetTreeModel::RecoverNewValuesToBackup(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int affectedRows{0};
  const QVector<int> affectedRoles{Qt::DisplayRole, Qt::DecorationRole};
  for (const QModelIndex& ind : indexes) {
    if (!ind.isValid()) {
      LOG_W("row[%d] out of range", ind.row());
      continue;
    }
    StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", ind.row());
      continue;
    }
    StyleItemData& item = node->value();
    if (!item.recoverToBackup()) {
      continue; // group: no need recover to backup
    }
    const QModelIndex editInd{siblingAtColumn(ind, StyleItemData::EDITABLE_COLUMN)};
    editCellEraseIndex(editInd);
    emit dataChanged(editInd, editInd, affectedRoles);
    ++affectedRows;
  }
  LOG_D("%d in %d new value field get reset to its backup", affectedRows, indexes.size());
  return affectedRows;
}

QVariantHash StyleSheetTreeModel::CollectItemsNeedApplyChange(const QModelIndexList& indexes) const {
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
    if (!item.isNeedApplyChange()) {
      continue;
    }
    QString cfgKey = node->GetConfigKey();
    key2Cfg[cfgKey] = item.modifiedToValue;
  }

  const int affectedRows{key2Cfg.size()};
  LOG_D("%d in %d duration field get updated", affectedRows, indexes.size());
  return key2Cfg;
}
