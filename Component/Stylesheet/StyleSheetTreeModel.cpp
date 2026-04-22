#include "StyleSheetTreeModel.h"
#include "StyleSheetGetter.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QVariantHash>
#include <QIcon>
#include <QColor>

StyleSheetTreeModel::StyleSheetTreeModel(QObject* parent) : QAbstractTreeModelPub<StyleTreeNode>{parent} {
  auto pRootUniquePtr = StyleSheetGetter::GetInst().GetModelData();
  {
    mFontGeneralFamilyNode = pRootUniquePtr->FindNode("StyleSheet/Font/Family/General");
    mFontGeneralSizeNode = pRootUniquePtr->FindNode("StyleSheet/Font/Size/General");
    mFontGeneralWeightNode = pRootUniquePtr->FindNode("StyleSheet/Font/Weight/General");
    mFontGeneralStyleNode = pRootUniquePtr->FindNode("StyleSheet/Font/Style/General");
  }
  setDatas(std::move(pRootUniquePtr));
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
      continue;  // group: no need clear
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
      continue;  // group: no need recover to default
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
      continue;  // group: no need recover to backup
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

  QVariantHash key2Cfg;
  for (const QModelIndex& ind : indexes) {
    if (!ind.isValid()) {
      LOG_W("row[%d] out of range", ind.row());
      continue;
    }
    const StyleTreeNode* node = static_cast<StyleTreeNode*>(ind.internalPointer());
    if (node == nullptr) {
      LOG_W("node in row[%d] is nullptr", ind.row());
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
