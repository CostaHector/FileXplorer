#include "LogProxyModel.h"

LogProxyModel::LogProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
  setFilterKeyColumn(1);
}

