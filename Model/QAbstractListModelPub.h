#ifndef QABSTRACTLISTMODELPUB_H
#define QABSTRACTLISTMODELPUB_H
#include <QAbstractListModel>
#include <stack>
#include <QPixmap>

class QAbstractListModelPub : public QAbstractListModel {
 public:
  using QAbstractListModel::QAbstractListModel;
  bool RowsCountBeginChange(int beforeRow, int afterRow);
  bool RowsCountEndChange();
  QPixmap GetDecorationPixmap(const QString& imgAbsPath) const;
  int getPixmapWidth() const { return mWidth; }
  int getPixmapHeight() const { return mHeight; }
  void setPixmapWidth(int newWidth) { mWidth = newWidth; }
  void setPixmapHeight(int newHeight) { mHeight = newHeight; }

 public slots:
  void onIconSizeChange(const QSize& newSize);

 private:
  struct DimensionRange {
    DimensionRange() = delete;
    DimensionRange(int bef, int aft) : m_bef{bef}, m_aft{aft} {}
    int m_bef;
    int m_aft;
  };
  bool IsDimensionCntValid(int bef, int aft) const { return bef >= 0 && aft >= 0; }
  std::stack<DimensionRange> mRowChangeStack;
  int mWidth = 404, mHeight = 250;
};

template <typename RandomAccessible1DimensionContainerDataType>
std::pair<bool, RandomAccessible1DimensionContainerDataType> MoveItemsBase(const RandomAccessible1DimensionContainerDataType& dataList, const QList<int>& ascUnqiueSelectRows, int targetRow);

#endif  // QABSTRACTLISTMODELPUB_H
