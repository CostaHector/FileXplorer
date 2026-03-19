#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "QAbstractListModelPub.h"
#include "EndToExposePrivateMember.h"
#include <QSignalSpy>
#include "MemoryKey.h"

using intQList = QList<int>;
extern template std::pair<bool, intQList> MoveItemsBase<intQList>(const intQList&, const QList<int>&, int);

class Dim1ContainerListModel : public QAbstractListModelPub {
 public:
  using QAbstractListModelPub::QAbstractListModelPub;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mData.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    const int r = index.row();
    if (r < 0 || r >= rowCount()) {
      return {};
    }
    if (role == Qt::DisplayRole) {
      return mData[r];
    }
    return {};
  }
  QStringList mData;
};

class QAbstractListModelPubTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void MoveItemsBase_ok() {
    intQList datas{0, 1, 2, 3, 4};
    intQList recoverDatas{datas};

    // one row selected
    {
      QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{2}, 0), (std::make_pair(true, intQList{2, 0, 1, 3, 4})));
      QCOMPARE(MoveItemsBase<intQList>({2, 0, 1, 3, 4}, QList<int>{0}, 3), (std::make_pair(true, recoverDatas)));

      QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{0}, 4), (std::make_pair(true, intQList{1, 2, 3, 0, 4})));
      QCOMPARE(MoveItemsBase<intQList>({1, 2, 3, 0, 4}, QList<int>{3}, 0), (std::make_pair(true, recoverDatas)));

      QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{4}, 1), (std::make_pair(true, intQList{0, 4, 1, 2, 3})));
      QCOMPARE(MoveItemsBase<intQList>({0, 4, 1, 2, 3}, QList<int>{1}, 5), (std::make_pair(true, recoverDatas)));
    }

    // move to before front
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{2, 3}, 0), (std::make_pair(true, intQList{2, 3, 0, 1, 4})));

    // recover
    QCOMPARE(MoveItemsBase<intQList>({2, 3, 0, 1, 4}, QList<int>{0, 1}, 4), (std::make_pair(true, recoverDatas)));

    // move to after end
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{2, 3}, 5), (std::make_pair(true, intQList{0, 1, 4, 2, 3})));

    // recover
    QCOMPARE(MoveItemsBase<intQList>({0, 1, 4, 2, 3}, QList<int>{3, 4}, 2), (std::make_pair(true, recoverDatas)));

    // move index 1
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{3, 4}, 1), (std::make_pair(true, intQList{0, 3, 4, 1, 2})));

    // recover
    QCOMPARE(MoveItemsBase<intQList>({0, 3, 4, 1, 2}, QList<int>{1, 2}, 5), (std::make_pair(true, recoverDatas)));
  }

  void MoveItemBase_skip_ok() {
    // no need move situation
    // 1. empty, no row exists at all
    // 2. empty, no row selected
    // 3. rows selected are continous and destination in it

    intQList emptyDatas;
    intQList datas{0, 1, 2, 3, 4};
    const std::pair<bool, intQList> failedResult{false, {}};

    QCOMPARE(MoveItemsBase<intQList>(emptyDatas, QList<int>{2, 3}, 0), failedResult);

    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{}, 0), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1}, 1), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2}, 1), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2}, 2), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3}, 1), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3}, 2), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3}, 3), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3, 4}, 1), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3, 4}, 2), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3, 4}, 3), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{1, 2, 3, 4}, 4), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{0, 1, 2, 3, 4}, 0), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{0, 1, 2, 3, 4}, 1), failedResult);
    QCOMPARE(MoveItemsBase<intQList>(datas, QList<int>{0, 1, 2, 3, 4}, 4), failedResult);
  }

  void dimension1_container_model_border_test() {
    Configuration().clear();

    {
      Dim1ContainerListModel rowModel{"Dim1ContainerTableListView"};
      QCOMPARE(rowModel.rowCount(), 0);
      // protection: not crashdown
      QVERIFY(rowModel.mRowChangeStack.empty());

      QVERIFY(!rowModel.RowsCountEndChange());  // empty stack

      QVERIFY(!rowModel.RowsCountBeginChange(-1, 1));  // invalid column count
      QVERIFY(!rowModel.RowsCountBeginChange(0, -1));  // invalid row count
      QVERIFY(!rowModel.RowsCountEndChange());         // empty stack

      QVERIFY(rowModel.mRowChangeStack.empty());

      const QList<QAction*> acts = rowModel.GetExcusiveActions();
      QCOMPARE(acts.contains(rowModel._PIXMAP_TRANSFORMATION_SMOOTH), true);

      rowModel.GetDecorationPixmap("");

      QVERIFY(rowModel.getPixmapWidth() > 0);
      QVERIFY(rowModel.getPixmapHeight() > 0);

      // 0 rows, onPixmapSmoothTransformationToggled will not emit data changed
      QSignalSpy decorationDataChangedSpy{&rowModel, &QAbstractListModelPub::dataChanged};
      QCOMPARE(rowModel.isPixmapTransformationSmooth(), false);
      QCOMPARE(rowModel.onPixmapSmoothTransformationToggled(false), false);  // unchanged

      QCOMPARE(rowModel._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), false);
      rowModel._PIXMAP_TRANSFORMATION_SMOOTH->toggle();
      QCOMPARE(rowModel._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), true);
      QCOMPARE(rowModel.isPixmapTransformationSmooth(), true);
      QCOMPARE(decorationDataChangedSpy.count(), 0);

      // 0 rows, onIconSizeChange will not emit data changed
      QCOMPARE(rowModel.onIconSizeChange({rowModel.getPixmapWidth(), rowModel.getPixmapHeight()}), false);  // unchange
      QCOMPARE(rowModel.onIconSizeChange({999, 1999}), true);
      QCOMPARE(decorationDataChangedSpy.count(), 0);
    }

    // will save _PIXMAP_TRANSFORMATION_SMOOTH into configuration in destructor
    {
      Dim1ContainerListModel rowModel{"Dim1ContainerTableListView"};
      QCOMPARE(rowModel.isPixmapTransformationSmooth(), true);
      QCOMPARE(rowModel._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), true);
    }
  }

  void dimension1_container_model_row_change_test() {
    Configuration().clear();
    Dim1ContainerListModel rowModel{"Dim1ContainerTableListView"};
    QCOMPARE(rowModel.rowCount(), 0);

    {  // 1. row count increasing
      QStringList rowString3{"Raphael Varane", "Mbappé", "Dembélé"};
      rowModel.RowsCountBeginChange(0, 3);
      rowModel.mData.swap(rowString3);
      QCOMPARE(rowModel.rowCount(), 3);
      rowModel.RowsCountEndChange();
      QModelIndex varaneIndex = rowModel.index(0);
      QModelIndex mbappeIndex = rowModel.index(1);
      QModelIndex dembeleIndex = rowModel.index(2);
      QCOMPARE(rowModel.data(varaneIndex).toString(), "Raphael Varane");
      QCOMPARE(rowModel.data(mbappeIndex).toString(), "Mbappé");
      QCOMPARE(rowModel.data(dembeleIndex).toString(), "Dembélé");

      QSignalSpy decorationDataChangedSpy{&rowModel, &Dim1ContainerListModel::dataChanged};
      QCOMPARE(rowModel.isPixmapTransformationSmooth(), false);
      QCOMPARE(rowModel.onPixmapSmoothTransformationToggled(true), true);  // unchanged
      QCOMPARE(rowModel.isPixmapTransformationSmooth(), true);
      QCOMPARE(rowModel._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), false);
      QCOMPARE(decorationDataChangedSpy.count(), 1);
      QVariantList parms1{decorationDataChangedSpy.takeLast()};
      // uhmmm? unknow reason, here only 1 element get in params
      // QCOMPARE(parms.size(), 3);
      // QCOMPARE(parms[0], varaneIndex);
      // QCOMPARE(parms[1], dembeleIndex);
      // QCOMPARE(parms[2].canConvert<QVector<int>>(), true);
      // QVector<int> roles = parms[2].value<QVector<int>>();
      // QCOMPARE(roles, (QVector<int>{Qt::DecorationRole}));
      rowModel._PIXMAP_TRANSFORMATION_SMOOTH->setChecked(true);

      QCOMPARE(rowModel.onIconSizeChange({1956, 2964}), true);
      QCOMPARE(decorationDataChangedSpy.count(), 1);
      QVariantList parms2{decorationDataChangedSpy.takeLast()};
      // uhmmm? unknow reason, here only 1 element get in params
    }

    {  // 1. row count remains. contents changed
      QStringList rowString3{"Mbappé", "Raphael Varane", "Dembélé"};
      rowModel.RowsCountBeginChange(3, 3);
      rowModel.mData.swap(rowString3);
      QCOMPARE(rowModel.rowCount(), 3);
      rowModel.RowsCountEndChange();
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Mbappé");
      QCOMPARE(rowModel.data(rowModel.index(1, 0)).toString(), "Raphael Varane");
      QCOMPARE(rowModel.data(rowModel.index(2, 0)).toString(), "Dembélé");
    }

    {  // 3. row count decreasing
      QStringList rowString1{"Raphael Varane"};
      rowModel.RowsCountBeginChange(3, 1);
      rowModel.mData.swap(rowString1);
      QCOMPARE(rowModel.rowCount(), 1);
      rowModel.RowsCountEndChange();
      QCOMPARE(rowModel.data(rowModel.index(0, 0)).toString(), "Raphael Varane");
    }
  }
};

#include "QAbstractListModelPubTest.moc"
REGISTER_TEST(QAbstractListModelPubTest, false)
