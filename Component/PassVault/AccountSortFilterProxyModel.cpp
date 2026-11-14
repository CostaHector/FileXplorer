#include "AccountSortFilterProxyModel.h"
#include "AccountTableModel.h"
#include "Bool2QtEnum.h"

bool AccountSortFilterProxyModel::initSortSetting(AccountInfo::Role initRole, bool bReverseOrder) {
  if (isSortProxyInited()) {
    return false;
  }
  setSortRole(initRole);
  mSortOrder = Bool2QtEnum::toSortOrder(bReverseOrder);
  sort(AccountInfo::SORT_COLUMN, mSortOrder);
  m_bSortProxyInited = true;
  return true;
}

bool AccountSortFilterProxyModel::setSortOrder(bool bReverseOrder) {
  const Qt::SortOrder newSortOrder{Bool2QtEnum::toSortOrder(bReverseOrder)};
  if (newSortOrder == mSortOrder) {
    return false;
  }
  mSortOrder = newSortOrder;
  sort(AccountInfo::SORT_COLUMN, mSortOrder);
  return true;
}

bool AccountSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const QString keyword = filterRegExp().pattern(); // Qt 5
#else
  const QString keyword = filterRegularExpression().pattern(); // Qt 6
#endif
  if (keyword.isEmpty()) {
    return true;
  }
  if (auto* pSrcModel = sourceModel()) {
    if (const auto* pAccountModel = static_cast<const AccountTableModel*>(pSrcModel)) {
      if (const AccountInfo* pInfo = pAccountModel->rowDataAt(sourceRow)) {
        return pInfo->IsContainsKeyWords(keyword);
      }
    }
  }
  return true;
}
