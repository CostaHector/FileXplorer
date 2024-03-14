#ifndef LOGPROXYMODEL_H
#define LOGPROXYMODEL_H

#include <QSortFilterProxyModel>

class LogProxyModel : public QSortFilterProxyModel {
 public:
  explicit LogProxyModel(QObject* parent = nullptr);


};

#endif  // LOGPROXYMODEL_H
