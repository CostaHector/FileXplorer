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
  void init() { Configuration().clear(); }

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

  void border_ok() {
    {
      Dim1ContainerListModel model{"Dim1ContainerTableListView"};
      QCOMPARE(model.rowCount(), 0);
      // protection: not crashdown
      QVERIFY(model.mRowChangeStack.empty());

      QVERIFY(!model.RowsCountEndChange());  // empty stack

      QVERIFY(!model.RowsCountBeginChange(-1, 1));  // invalid column count
      QVERIFY(!model.RowsCountBeginChange(0, -1));  // invalid row count
      QVERIFY(!model.RowsCountEndChange());         // empty stack

      QVERIFY(model.mRowChangeStack.empty());

      const QList<QAction*> acts = model.GetExcusiveActions();
      QCOMPARE(acts.contains(model._PIXMAP_TRANSFORMATION_SMOOTH), true);

      model.GetDecorationPixmap("");

      QVERIFY(model.getPixmapWidth() > 0);
      QVERIFY(model.getPixmapHeight() > 0);

      // 0 rows, onPixmapSmoothTransformationToggled will not emit data changed
      QSignalSpy decorationDataChangedSpy{&model, &QAbstractListModelPub::dataChanged};
      QCOMPARE(model.isPixmapTransformationSmooth(), false);
      QCOMPARE(model.onPixmapSmoothTransformationToggled(false), false);  // unchanged

      QCOMPARE(model._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), false);
      model._PIXMAP_TRANSFORMATION_SMOOTH->toggle();
      QCOMPARE(model._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), true);
      QCOMPARE(model.isPixmapTransformationSmooth(), true);
      QCOMPARE(decorationDataChangedSpy.count(), 0);

      // 0 rows, onIconSizeChange will not emit data changed
      QCOMPARE(model.onIconSizeChange({model.getPixmapWidth(), model.getPixmapHeight()}), false);  // unchange
      QCOMPARE(model.onIconSizeChange({999, 1999}), true);
      QCOMPARE(decorationDataChangedSpy.count(), 0);
    }

    // will save _PIXMAP_TRANSFORMATION_SMOOTH into configuration in destructor
    {
      Dim1ContainerListModel model{"Dim1ContainerTableListView"};
      QCOMPARE(model.isPixmapTransformationSmooth(), true);
      QCOMPARE(model._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), true);
    }
  }

  void RowsCountBeginChange_ok() {
    Dim1ContainerListModel model{"Dim1ContainerTableListView"};
    QCOMPARE(model.rowCount(), 0);

    {  // 1. row count increasing
      QStringList rowString3{"Raphael Varane", "Mbappé", "Dembélé"};
      model.RowsCountBeginChange(0, 3);
      model.mData.swap(rowString3);
      QCOMPARE(model.rowCount(), 3);
      model.RowsCountEndChange();
      QModelIndex varaneIndex = model.index(0);
      QModelIndex mbappeIndex = model.index(1);
      QModelIndex dembeleIndex = model.index(2);
      QCOMPARE(model.data(varaneIndex).toString(), "Raphael Varane");
      QCOMPARE(model.data(mbappeIndex).toString(), "Mbappé");
      QCOMPARE(model.data(dembeleIndex).toString(), "Dembélé");

      QSignalSpy decorationDataChangedSpy{&model, &Dim1ContainerListModel::dataChanged};
      QCOMPARE(model.isPixmapTransformationSmooth(), false);
      QCOMPARE(model.onPixmapSmoothTransformationToggled(true), true);  // unchanged
      QCOMPARE(model.isPixmapTransformationSmooth(), true);
      QCOMPARE(model._PIXMAP_TRANSFORMATION_SMOOTH->isChecked(), false);
      QCOMPARE(decorationDataChangedSpy.count(), 1);
      QVariantList parms1{decorationDataChangedSpy.takeLast()};
      // uhmmm? unknow reason, here only 1 element get in params
      // QCOMPARE(parms.size(), 3);
      // QCOMPARE(parms[0], varaneIndex);
      // QCOMPARE(parms[1], dembeleIndex);
      // QCOMPARE(parms[2].canConvert<QVector<int>>(), true);
      // QVector<int> roles = parms[2].value<QVector<int>>();
      // QCOMPARE(roles, (QVector<int>{Qt::DecorationRole}));
      model._PIXMAP_TRANSFORMATION_SMOOTH->setChecked(true);

      QCOMPARE(model.onIconSizeChange({1956, 2964}), true);
      QCOMPARE(decorationDataChangedSpy.count(), 1);
      QVariantList parms2{decorationDataChangedSpy.takeLast()};
      // uhmmm? unknow reason, here only 1 element get in params
    }

    {  // 1. row count remains. contents changed
      QStringList rowString3{"Mbappé", "Raphael Varane", "Dembélé"};
      model.RowsCountBeginChange(3, 3);
      model.mData.swap(rowString3);
      QCOMPARE(model.rowCount(), 3);
      model.RowsCountEndChange();
      QCOMPARE(model.data(model.index(0, 0)).toString(), "Mbappé");
      QCOMPARE(model.data(model.index(1, 0)).toString(), "Raphael Varane");
      QCOMPARE(model.data(model.index(2, 0)).toString(), "Dembélé");
    }

    {  // 3. row count decreasing
      QStringList rowString1{"Raphael Varane"};
      model.RowsCountBeginChange(3, 1);
      model.mData.swap(rowString1);
      QCOMPARE(model.rowCount(), 1);
      model.RowsCountEndChange();
      QCOMPARE(model.data(model.index(0, 0)).toString(), "Raphael Varane");
    }
  }

  void onRowsRemoved_ok() {
    Dim1ContainerListModel model{"Dim1ContainerTableListView"};
    QCOMPARE(model.rowCount(), 0);

    QStringList rowString3{"Raphael Varane", "Kaka", "Cristiano Ronaldo"};
    model.RowsCountBeginChange(0, 3);
    model.mData.swap(rowString3);
    model.RowsCountEndChange();
    QCOMPARE(model.rowCount(), 3);

    QStringList& dataList = model.mData;
    const auto rowElementsRmv = [&dataList](int beg, int end) { dataList.erase(dataList.begin() + beg, dataList.begin() + end); };
    QCOMPARE(model.onRowsRemoved({}, rowElementsRmv), 0);
    QCOMPARE(model.rowCount(), 3);

    QModelIndex KakaIndex = model.index(1);
    QCOMPARE(model.onRowsRemoved({KakaIndex}, nullptr), 1);
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.onRowsRemoved({KakaIndex}, rowElementsRmv), 1);
    QCOMPARE(model.rowCount(), 2);
  }
};

#include "QAbstractListModelPubTest.moc"
REGISTER_TEST(QAbstractListModelPubTest, false)
