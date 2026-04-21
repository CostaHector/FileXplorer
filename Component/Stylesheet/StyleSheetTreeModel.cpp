#include "StyleSheetTreeModel.h"
#include "StyleSheetGetter.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QVariantHash>
#include <QIcon>
#include <QColor>

StyleSheetTreeModel::StyleSheetTreeModel(QObject* parent)
  : QAbstractTreeModelPub<StyleTreeNode>{parent} {
  std::unique_ptr<StyleTreeNode> pRoot{StyleTreeNode::NewTreeNodeRoot("StyleSheet")};
  initColorRelated(pRoot, Style::StyleSheetE::STYLESHEET_LIGHT);
  initColorRelated(pRoot, Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG);
  initFontRelated(pRoot);
  setDatas(std::move(pRoot));
}

bool StyleSheetTreeModel::initFontRelated(std::unique_ptr<StyleTreeNode>& pRoot) const {
  if (!pRoot) {
    return false;
  }
  using namespace FontCfg;
  auto* pFont = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
  {
    auto* pFontFamily = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Family"}));
    {
      mFontGeneralFamilyNode = pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontFamilyDef, GetFontFamily(), StyleItemData::FONT_FAMILY}));
      pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"Code", mFontFamilyCodeDef, GetFontFamilyCode(), StyleItemData::FONT_FAMILY}));
    }

    auto* pFontSize = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Size"}));
    {
      mFontGeneralSizeNode = pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontSizeDef, GetFontSize(), StyleItemData::NUMBER}));
      pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"QTabBar", mFontSizeDef, GetFontSizeTab(), StyleItemData::NUMBER}));
    }

    auto* pFontWeight = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Weight"}));
    {
      mFontGeneralWeightNode = pFontWeight->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontWeightDef, GetFontWeight(), StyleItemData::FONT_WEIGHT}));
    }

    auto* pFontStyle = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Style"}));
    {
      mFontGeneralStyleNode = pFontStyle->appendRow(StyleTreeNode::create(StyleItemData{"General", mFontStyleDef, GetFontStyle(), StyleItemData::FONT_STYLE}));
    }
  }
  return true;
}

bool StyleSheetTreeModel::initColorRelated(std::unique_ptr<StyleTreeNode>& pRoot, Style::StyleSheetE styleE) const {
  if (!pRoot) {
    return false;
  }
  const auto& inst = StyleSheetGetter::GetInst();
  QString prefixKeyName;

  using namespace Style;
  const QString styleName{styleE == StyleSheetE::STYLESHEET_LIGHT ? "LightColor" : "DarkColor"};
  auto* pColor = pRoot->appendRow(StyleTreeNode::create(StyleItemData{styleName}));

  auto* pColorBg = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Background"}));
  {
    prefixKeyName = pColorBg->GetConfigKey();
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"General", inst.defCurValue(prefixKeyName + "/General"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"Disabled", inst.defCurValue(prefixKeyName + "/Disabled"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"Label", inst.defCurValue(prefixKeyName + "/Label"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"LineEdit", inst.defCurValue(prefixKeyName + "/LineEdit"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"ComboBox", inst.defCurValue(prefixKeyName + "/ComboBox"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"ToolBar", inst.defCurValue(prefixKeyName + "/ToolBar"), StyleItemData::COLOR}));
    pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"StatusBar", inst.defCurValue(prefixKeyName + "/StatusBar"), StyleItemData::COLOR}));

    auto* tabBar = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"TabBar"}));
    {
      prefixKeyName = tabBar->GetConfigKey();
      tabBar->appendRow(StyleTreeNode::create(StyleItemData{"tabNotSelected", inst.defCurValue(prefixKeyName + "/tabNotSelected"), StyleItemData::COLOR}));
      tabBar->appendRow(StyleTreeNode::create(StyleItemData{"tabSelected", inst.defCurValue(prefixKeyName + "/tabSelected"), StyleItemData::COLOR}));
      tabBar->appendRow(StyleTreeNode::create(StyleItemData{"tabHover", inst.defCurValue(prefixKeyName + "/tabHover"), StyleItemData::COLOR}));
    }

    auto* menu = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"Menu"}));
    {
      prefixKeyName = menu->GetConfigKey();
      menu->appendRow(StyleTreeNode::create(StyleItemData{"Item", inst.defCurValue(prefixKeyName + "/Item"), StyleItemData::COLOR}));
      menu->appendRow(StyleTreeNode::create(StyleItemData{"ItemChecked", inst.defCurValue(prefixKeyName + "/ItemChecked"), StyleItemData::COLOR}));
      menu->appendRow(StyleTreeNode::create(StyleItemData{"ItemHovered", inst.defCurValue(prefixKeyName + "/ItemHovered"), StyleItemData::COLOR}));
      menu->appendRow(StyleTreeNode::create(StyleItemData{"Separator", inst.defCurValue(prefixKeyName + "/Separator"), StyleItemData::COLOR}));
      menu->appendRow(StyleTreeNode::create(StyleItemData{"ItemRightBorder", inst.defCurValue(prefixKeyName + "/ItemRightBorder"), StyleItemData::COLOR}));
      menu->appendRow(StyleTreeNode::create(StyleItemData{"Border", inst.defCurValue(prefixKeyName + "/Border"), StyleItemData::COLOR}));
    }

    auto* view = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"View"}));
    {
      prefixKeyName = view->GetConfigKey();
      view->appendRow(StyleTreeNode::create(StyleItemData{"Panel", inst.defCurValue(prefixKeyName + "/Panel"), StyleItemData::COLOR}));
      view->appendRow(StyleTreeNode::create(StyleItemData{"AlternateRow", inst.defCurValue(prefixKeyName + "/AlternateRow"), StyleItemData::COLOR}));
      view->appendRow(StyleTreeNode::create(StyleItemData{"SelectedActive", inst.defCurValue(prefixKeyName + "/SelectedActive"), StyleItemData::COLOR}));
      view->appendRow(StyleTreeNode::create(StyleItemData{"SelectedInActive", inst.defCurValue(prefixKeyName + "/SelectedInActive"), StyleItemData::COLOR}));
      view->appendRow(StyleTreeNode::create(StyleItemData{"Hover", inst.defCurValue(prefixKeyName + "/Hover"), StyleItemData::COLOR}));
      view->appendRow(StyleTreeNode::create(StyleItemData{"Gridline", inst.defCurValue(prefixKeyName + "/Gridline"), StyleItemData::COLOR}));

      auto* tableCornerButton = view->appendRow(StyleTreeNode::create(StyleItemData{"TableCornerButton"}));
      {
        prefixKeyName = tableCornerButton->GetConfigKey();
        tableCornerButton->appendRow(StyleTreeNode::create(StyleItemData{"Section", inst.defCurValue(prefixKeyName + "/Section"), StyleItemData::COLOR}));
      }
    }

    auto* headerView = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"HeaderView"}));
    {
      prefixKeyName = headerView->GetConfigKey();
      headerView->appendRow(StyleTreeNode::create(StyleItemData{"NotHover", inst.defCurValue(prefixKeyName + "/NotHover"), StyleItemData::COLOR}));
      headerView->appendRow(StyleTreeNode::create(StyleItemData{"Hover", inst.defCurValue(prefixKeyName + "/Hover"), StyleItemData::COLOR}));
      headerView->appendRow(StyleTreeNode::create(StyleItemData{"Clicked", inst.defCurValue(prefixKeyName + "/Pressed"), StyleItemData::COLOR}));
    }

    auto* scrollBar = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"ScrollBar"}));
    {
      prefixKeyName = scrollBar->GetConfigKey();
      scrollBar->appendRow(StyleTreeNode::create(StyleItemData{"EmptyZone", inst.defCurValue(prefixKeyName + "/EmptyZone"), StyleItemData::COLOR}));
      scrollBar->appendRow(StyleTreeNode::create(StyleItemData{"Handle", inst.defCurValue(prefixKeyName + "/Handle"), StyleItemData::COLOR}));
      scrollBar->appendRow(StyleTreeNode::create(StyleItemData{"Checked", inst.defCurValue(prefixKeyName + "/Checked"), StyleItemData::COLOR}));
    }

    auto* abstractButton = pColorBg->appendRow(StyleTreeNode::create(StyleItemData{"AbstractButton"}));
    {
      auto* pushButton = abstractButton->appendRow(StyleTreeNode::create(StyleItemData{"PushButton"}));
      {
        prefixKeyName = pushButton->GetConfigKey();
        pushButton->appendRow(StyleTreeNode::create(StyleItemData{"General", inst.defCurValue(prefixKeyName + "/General"), StyleItemData::COLOR}));
        pushButton->appendRow(StyleTreeNode::create(StyleItemData{"Hovered", inst.defCurValue(prefixKeyName + "/Hovered"), StyleItemData::COLOR}));
        pushButton->appendRow(StyleTreeNode::create(StyleItemData{"Pressed", inst.defCurValue(prefixKeyName + "/Pressed"), StyleItemData::COLOR}));
        pushButton->appendRow(StyleTreeNode::create(StyleItemData{"Checked", inst.defCurValue(prefixKeyName + "/Checked"), StyleItemData::COLOR}));
      }
      auto* toolButton = abstractButton->appendRow(StyleTreeNode::create(StyleItemData{"ToolButton"}));
      {
        prefixKeyName = toolButton->GetConfigKey();
        toolButton->appendRow(StyleTreeNode::create(StyleItemData{"General", inst.defCurValue(prefixKeyName + "/General"), StyleItemData::COLOR}));
        toolButton->appendRow(StyleTreeNode::create(StyleItemData{"Hovered", inst.defCurValue(prefixKeyName + "/Hovered"), StyleItemData::COLOR}));
        toolButton->appendRow(StyleTreeNode::create(StyleItemData{"Pressed", inst.defCurValue(prefixKeyName + "/Pressed"), StyleItemData::COLOR}));
        toolButton->appendRow(StyleTreeNode::create(StyleItemData{"Checked", inst.defCurValue(prefixKeyName + "/Checked"), StyleItemData::COLOR}));
      }
    }
  }

  auto* pColorBorder = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Border"}));
  {
    prefixKeyName = pColorBorder->GetConfigKey();
    pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"General", inst.defCurValue(prefixKeyName + "/General"), StyleItemData::COLOR}));
    pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"ComboBox", inst.defCurValue(prefixKeyName + "/ComboBox"), StyleItemData::COLOR}));
    pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"TabWidget", inst.defCurValue(prefixKeyName + "/TabWidget"), StyleItemData::COLOR}));
    pColorBorder->appendRow(StyleTreeNode::create(StyleItemData{"HeaderView", inst.defCurValue(prefixKeyName + "/HeaderView"), StyleItemData::COLOR}));
  }

  auto* pColorForeground = pColor->appendRow(StyleTreeNode::create(StyleItemData{"Foreground"}));
  {
    prefixKeyName = pColorForeground->GetConfigKey();
    pColorForeground->appendRow(StyleTreeNode::create(StyleItemData{"General", inst.defCurValue(prefixKeyName + "/General"), StyleItemData::COLOR}));
    pColorForeground->appendRow(StyleTreeNode::create(StyleItemData{"Disabled", inst.defCurValue(prefixKeyName + "/Disabled"), StyleItemData::COLOR}));
    pColorForeground->appendRow(StyleTreeNode::create(StyleItemData{"Placeholder", inst.defCurValue(prefixKeyName + "/Placeholder"), StyleItemData::COLOR}));
    pColorForeground->appendRow(StyleTreeNode::create(StyleItemData{"MenuFont", inst.defCurValue(prefixKeyName + "/MenuFont"), StyleItemData::COLOR}));
  }

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
  if (m_bLivePreviewSwitch) {
    QString cfgKey = node->GetConfigKey();
    emit requestSeeChanges(cfgKey, value);
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

int StyleSheetTreeModel::SetFontGeneral(const QFont& newGeneralFont) {
  CHECK_NULLPTR_RETURN_INT(mFontGeneralFamilyNode, -1);
  CHECK_NULLPTR_RETURN_INT(mFontGeneralSizeNode, -1);
  CHECK_NULLPTR_RETURN_INT(mFontGeneralWeightNode, -1);
  CHECK_NULLPTR_RETURN_INT(mFontGeneralStyleNode, -1);

  int attributeChangedCnt{0};
  attributeChangedCnt += setData(siblingAtColumn(indexFromItem(mFontGeneralFamilyNode), StyleItemData::EDITABLE_COLUMN), newGeneralFont.family(), Qt::EditRole);
  attributeChangedCnt += setData(siblingAtColumn(indexFromItem(mFontGeneralSizeNode), StyleItemData::EDITABLE_COLUMN), newGeneralFont.pointSize(), Qt::EditRole);
  attributeChangedCnt += setData(siblingAtColumn(indexFromItem(mFontGeneralWeightNode), StyleItemData::EDITABLE_COLUMN), newGeneralFont.weight(), Qt::EditRole);
  attributeChangedCnt += setData(siblingAtColumn(indexFromItem(mFontGeneralStyleNode), StyleItemData::EDITABLE_COLUMN), newGeneralFont.style(), Qt::EditRole);
  return attributeChangedCnt;
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

QVariantHash StyleSheetTreeModel::CollectItemsNeedSeeChange(const QModelIndexList& indexes) const {
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
