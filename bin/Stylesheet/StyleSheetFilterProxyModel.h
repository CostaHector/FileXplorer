#ifndef STYLESHEETFILTERPROXYMODEL_H
#define STYLESHEETFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class StyleSheetFilterProxyModel : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  void setFilterString(const QString& filter);
  QString curFilter() const { return m_filter; }

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  QString m_filter;
  mutable QHash<const void*, bool> mPassCache;
};

#endif // STYLESHEETFILTERPROXYMODEL_H
