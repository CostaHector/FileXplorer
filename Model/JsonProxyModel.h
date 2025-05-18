#ifndef JSONPROXYMODEL_H
#define JSONPROXYMODEL_H

#include <QSortFilterProxyModel>

class JsonProxyModel : public QSortFilterProxyModel {
 public:
  explicit JsonProxyModel(QObject* parent = nullptr);

  void invalidateFilterPublic() { invalidateFilter(); }

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
};

#endif  // JSONPROXYMODEL_H
