#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "ModelTools.h"

#include "BeginToExposePrivateMember.h"
#include "QAbstractTableModelPub.h"
#include "QAbstractListModelPub.h"
#include "EndToExposePrivateMember.h"

template <typename Swappable2DimContainerDataType>
class Dim2ContainerTableModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mData.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return mData.isEmpty() ? 0 : mData[0].size(); }
  std::pair<int, int> dimension() const { return {rowCount(), columnCount()}; }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    const int r = index.row(), c = index.column();
    if (r < 0 || r >= rowCount()) {
      LOG_W("r:%d is out of range[0, %d)", r, rowCount());
      return {};
    }
    if (c < 0 || c >= std::min(columnCount(), mData[r].size())) {
      LOG_W("c:%d is out of range[0, min(%d, %d))", c, columnCount(), mData[r].size());
      return {};
    }

    if (role == Qt::DisplayRole) {
      return mData[r][c];
    }
    return {};
  }
  Swappable2DimContainerDataType mData;
};

using SWAPPABLE_STRINGLIST_LIST = QList<QStringList>;

extern template void QAbstractTableModelPub::DimensionCountChange<SWAPPABLE_STRINGLIST_LIST>(SWAPPABLE_STRINGLIST_LIST&,
                                                                                             SWAPPABLE_STRINGLIST_LIST&,
                                                                                             const QAbstractTableModelPub::DataChangeRangeE);

class QAbstractTableModelPubTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void MergeList2SectionsRange_ok() {
    QCOMPARE(ModelTools::MergeList2SectionsRange({}), (QList<std::pair<int, int>>{}));
    QCOMPARE(ModelTools::MergeList2SectionsRange({0, 1, 2, 3}), (QList<std::pair<int, int>>{{0, 3}}));
    QCOMPARE(ModelTools::MergeList2SectionsRange({1, 3, 4}), (QList<std::pair<int, int>>{{1, 1}, {3, 4}}));
    QCOMPARE(ModelTools::MergeList2SectionsRange({0, 1, 2, 4}), (QList<std::pair<int, int>>{{0, 2}, {4, 4}}));
  }

  void dimesion2_container_model_border_test() {
    Dim2ContainerTableModel<SWAPPABLE_STRINGLIST_LIST> rowModel;
    QCOMPARE(rowModel.dimension(), (std::pair<int, int>(0, 0)));
    {  // protection should not crashdown
      QVERIFY(rowModel.mRowChangeStack.empty());
      QVERIFY(rowModel.mColumnChangeStack.empty());

      QVERIFY(!rowModel.RowsCountEndChange());     // empty stack
      QVERIFY(!rowModel.ColumnsCountEndChange());  // empty stack

      QVERIFY(!rowModel.RowsCountBeginChange(-1, 1));  // invalid column count
      QVERIFY(!rowModel.RowsCountBeginChange(0, -1));  // invalid row count
      QVERIFY(!rowModel.RowsCountEndChange());         // empty stack

      QVERIFY(!rowModel.ColumnsCountBeginChange(-1, 1));  // invalid column count
      QVERIFY(!rowModel.ColumnsCountBeginChange(0, -1));  // invalid column count
      QVERIFY(!rowModel.ColumnsCountEndChange());         // empty stack

      QVERIFY(rowModel.mRowChangeStack.empty());
      QVERIFY(rowModel.mColumnChangeStack.empty());
    }
  }

  void onRowRemoved_ok() {
    Dim2ContainerTableModel<SWAPPABLE_STRINGLIST_LIST> rowModel;
    QCOMPARE(rowModel.dimension(), (std::pair<int, int>(0, 0)));
    SWAPPABLE_STRINGLIST_LIST afterStringList31{
        // 3-by-1
        {"Cristiano Ronaldo"},  //
        {"Leite Ricardo"},
        {"Robert Lewandowski"},
    };
    rowModel.RowsCountBeginChange(0, 3);
    rowModel.mData.swap(afterStringList31);
    rowModel.RowsCountEndChange();
    QCOMPARE(rowModel.dimension(), (std::pair<int, int>(3, 1)));  // 3*1

    const auto rowElementsRmv = [&rowModel](int beg, int end) { rowModel.mData.erase(rowModel.mData.begin() + beg, rowModel.mData.begin() + end); };
    QCOMPARE(rowModel.onRowsRemoved({}, rowElementsRmv), 0);                                                   // no index
    QCOMPARE(rowModel.onRowsRemoved({QModelIndex{}}, rowElementsRmv), 0);                               // no valid index
    QCOMPARE(rowModel.onRowsRemoved({rowModel.index(1, 0), rowModel.index(0, 0)}, rowElementsRmv), 2);  // 2 rows removed
    QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 1)));
  }

  void dimesion2_container_model_change_ok() {
    Dim2ContainerTableModel<SWAPPABLE_STRINGLIST_LIST> rowModel;
    QCOMPARE(rowModel.dimension(), (std::pair<int, int>(0, 0)));
    {
      // 1. row change
      SWAPPABLE_STRINGLIST_LIST afterStringList31{
          // 3-by-1
          {"Cristiano Ronaldo"},  //
          {"Kaka"},
          {"Robert Lewandowski"},
      };
      rowModel.RowsCountBeginChange(0, 3);
      rowModel.mData.swap(afterStringList31);
      rowModel.RowsCountEndChange();
      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(3, 1)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Cristiano Ronaldo");
      QCOMPARE(rowModel.data(rowModel.index(1, 0)).toString(), "Kaka");
      QCOMPARE(rowModel.data(rowModel.index(2, 0)).toString(), "Robert Lewandowski");

      // row descrease
      SWAPPABLE_STRINGLIST_LIST afterStringList11{{"Raphael Varane"}};  // 1-by-1
      rowModel.RowsCountBeginChange(3, 1);
      rowModel.mData.swap(afterStringList11);
      rowModel.RowsCountEndChange();

      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 1)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Raphael Varane");
    }

    {
      // 2. column change
      SWAPPABLE_STRINGLIST_LIST afterStringList13{{"Raphael Varane", "Mbappé", "Dembélé"}};  // 1-by-1
      rowModel.ColumnsCountBeginChange(1, 3);
      rowModel.mData.swap(afterStringList13);
      rowModel.ColumnsCountEndChange();

      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 3)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Raphael Varane");
      QCOMPARE(rowModel.data(rowModel.index(0, 1)).toString(), "Mbappé");
      QCOMPARE(rowModel.data(rowModel.index(0, 2)).toString(), "Dembélé");

      SWAPPABLE_STRINGLIST_LIST afterStringList11{{"Mbappé"}};  // 1-by-1
      rowModel.ColumnsCountBeginChange(1, 1);
      rowModel.mData.swap(afterStringList11);
      rowModel.ColumnsCountEndChange();

      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 1)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Mbappé");
    }

    {  // dimension change
      SWAPPABLE_STRINGLIST_LIST afterStringList33{
          // 3-by-3
          {"Cristiano Ronaldo", "Bernardo Silva", "Ruben Dias"},  //
          {"Kai Havertz", "Marc-André ter Stegen"},               //
          {"Robert Lewandowski"},                                 //
      };

      // both row and column changed. access out of range index valid
      rowModel.DimensionCountChange(rowModel.mData, afterStringList33, QAbstractTableModelPub::DataChangeRangeE::BOTH_ROW_AND_COLUMN);
      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(3, 3)));

      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Cristiano Ronaldo");
      QCOMPARE(rowModel.data(rowModel.index(1, 0)).toString(), "Kai Havertz");
      QCOMPARE(rowModel.data(rowModel.index(2, 0)).toString(), "Robert Lewandowski");

      QCOMPARE(rowModel.data(rowModel.index(0, 1)).toString(), "Bernardo Silva");
      QCOMPARE(rowModel.data(rowModel.index(1, 1)).toString(), "Marc-André ter Stegen");
      QCOMPARE(rowModel.data(rowModel.index(2, 1)).toString(), "");  // out of range protection, should not crash down

      QCOMPARE(rowModel.data(rowModel.index(0, 2)).toString(), "Ruben Dias");
      QCOMPARE(rowModel.data(rowModel.index(1, 2)).toString(), "");  // out of range protection
      QCOMPARE(rowModel.data(rowModel.index(2, 2)).toString(), "");  // out of range protection

      // only row changed
      SWAPPABLE_STRINGLIST_LIST afterStringList13{
          // 1-by-3
          {"Ruben Dias", "Cristiano Ronaldo", "Bernardo Silva"},  //
      };
      rowModel.DimensionCountChange(rowModel.mData, afterStringList13, QAbstractTableModelPub::DataChangeRangeE::ROW);
      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 3)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Ruben Dias");
      QCOMPARE(rowModel.data(rowModel.index(0, 1)).toString(), "Cristiano Ronaldo");
      QCOMPARE(rowModel.data(rowModel.index(0, 2)).toString(), "Bernardo Silva");

      // only column change
      SWAPPABLE_STRINGLIST_LIST afterStringList14{
          // 1-by-4
          {"Ruben Dias", "Cristiano Ronaldo", "Pepe", "Bernardo Silva"},  //
      };
      rowModel.DimensionCountChange(rowModel.mData, afterStringList14, QAbstractTableModelPub::DataChangeRangeE::COLUMN);
      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(1, 4)));
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Ruben Dias");
      QCOMPARE(rowModel.data(rowModel.index(0, 1)).toString(), "Cristiano Ronaldo");
      QCOMPARE(rowModel.data(rowModel.index(0, 2)).toString(), "Pepe");
      QCOMPARE(rowModel.data(rowModel.index(0, 3)).toString(), "Bernardo Silva");

      // change to 0-by-0 ok
      SWAPPABLE_STRINGLIST_LIST afterStringList00;
      rowModel.DimensionCountChange(rowModel.mData, afterStringList00, QAbstractTableModelPub::DataChangeRangeE::COLUMN);
      QCOMPARE(rowModel.dimension(), (std::pair<int, int>(0, 0)));
      QCOMPARE(rowModel.data(rowModel.index(1, 1)).toString(), "");   // should not crash down
      QCOMPARE(rowModel.data(rowModel.index(0, -1)).toString(), "");  // should not crash down
      QCOMPARE(rowModel.data(rowModel.index(0, 1)).toString(), "");   // should not crash down
      QCOMPARE(rowModel.data(rowModel.index(-1, 0)).toString(), "");  // should not crash down
    }
  }
};

#include "QAbstractTableModelPubTest.moc"
REGISTER_TEST(QAbstractTableModelPubTest, false)
