#ifndef QABSTRACTLISTMODELPUB_H
#define QABSTRACTLISTMODELPUB_H
#include <QAbstractListModel>
#include <stack>
#include <QPixmap>
#include <QAction>

class QAbstractListModelPub : public QAbstractListModel {
 public:
  explicit QAbstractListModelPub(const QString& listViewName, QObject *parent = nullptr);
  ~QAbstractListModelPub();

  bool RowsCountBeginChange(int beforeRow, int afterRow);
  bool RowsCountEndChange();
  QPixmap GetDecorationPixmap(const QString& imgAbsPath) const;
  int getPixmapWidth() const { return mWidth; }
  int getPixmapHeight() const { return mHeight; }
  void setPixmapWidth(int newWidth) { mWidth = newWidth; }
  void setPixmapHeight(int newHeight) { mHeight = newHeight; }
  inline bool isPixmapTransformationSmooth() const { return m_isSmooth;}
  QString GetName() const { return mMemoryName; }

  QList<QAction*> GetExcusiveActions() const;

 public slots:
  void onIconSizeChange(const QSize& newSize);
  void onPixmapSmoothTransformationToggled(bool bSmooth);

 private:
  void subscribe();
  struct DimensionRange {
    DimensionRange() = delete;
    DimensionRange(int bef, int aft) : m_bef{bef}, m_aft{aft} {}
    int m_bef;
    int m_aft;
  };
  bool IsDimensionCntValid(int bef, int aft) const { return bef >= 0 && aft >= 0; }
  std::stack<DimensionRange> mRowChangeStack;
  int mWidth = 404, mHeight = 250;

  QAction* _PIXMAP_TRANSFORMATION_SMOOTH{nullptr};
  bool m_isSmooth{false};
  const QString mMemoryName;
};

template <typename RandomAccessible1DimensionContainerDataType>
std::pair<bool, RandomAccessible1DimensionContainerDataType> MoveItemsBase(const RandomAccessible1DimensionContainerDataType& dataList, const QList<int>& ascUnqiueSelectRows, int targetRow);

#endif  // QABSTRACTLISTMODELPUB_H
