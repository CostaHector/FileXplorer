#ifndef ACCOUNTSORTFILTERPROXYMODEL_H
#define ACCOUNTSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "AccountInfo.h"

class AccountSortFilterProxyModel : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  bool isSortProxyInited() const { return m_bSortProxyInited; }
  bool initSortSetting(AccountInfo::Role initRole, bool bReverseOrder);
  bool setSortOrder(bool bReverseOrder);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  Qt::SortOrder mSortOrder{Qt::SortOrder::AscendingOrder};
  bool m_bSortProxyInited{false};
};

#endif // ACCOUNTSORTFILTERPROXYMODEL_H
