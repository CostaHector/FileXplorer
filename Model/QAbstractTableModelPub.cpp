#include "QAbstractTableModelPub.h"

template<typename SwappableContainerDataType>
void QAbstractTableModelPub::RowsCountChange(SwappableContainerDataType& lhs, SwappableContainerDataType& rhs) {
  const int beforeCnt = lhs.size(), afterCnt = rhs.size();
  RowsCountBeginChange(beforeCnt, afterCnt);
  lhs.swap(rhs);
  RowsCountEndChange();
}
template void QAbstractTableModelPub::RowsCountChange<QStringList>(QStringList&, QStringList&);
