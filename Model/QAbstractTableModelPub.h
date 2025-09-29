#ifndef QABSTRACTTABLEMODELPUB_H
#define QABSTRACTTABLEMODELPUB_H

#include <QAbstractTableModel>
#include <stack>
class QAbstractTableModelPub : public QAbstractTableModel {
 public:
  using QAbstractTableModel::QAbstractTableModel;

  bool RowsCountBeginChange(int beforeRow, int afterRow);
  bool RowsCountEndChange(); // must call me after RowsCountBeginChange called and data change finished
  bool ColumnsCountBeginChange(int beforeColumnCnt, int afterColumnCnt);
  bool ColumnsCountEndChange(); // must call me after ColumnsCountBeginChange called and data change finished

  enum class DataChangeRangeE {
    ROW = 0x1 << 0,
    COLUMN = 0x1 << 1,
    BOTH_ROW_AND_COLUMN = (int)DataChangeRangeE::ROW | (int)DataChangeRangeE::COLUMN,
  };
  template <typename Swappable2DimContainerDataType>
  void DimensionCountChange(Swappable2DimContainerDataType& lhs, Swappable2DimContainerDataType& rhs, const DataChangeRangeE changeRangeE=DataChangeRangeE::ROW);

 private:
  struct DimensionRange {
    DimensionRange() = delete;
    DimensionRange(int bef, int aft) : m_bef{bef}, m_aft{aft} {}
    int m_bef;
    int m_aft;
  };
  static bool IsDimensionCntValid(int bef, int aft) { return bef >= 0 && aft >= 0; }
  std::stack<DimensionRange> mRowChangeStack, mColumnChangeStack;
};

#endif  // QABSTRACTTABLEMODELPUB_H
