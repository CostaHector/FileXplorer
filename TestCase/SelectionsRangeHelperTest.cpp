#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "SelectionsRangeHelper.h"
#include "EndToExposePrivateMember.h"

#include <QAbstractItemModel>

class TestModel : public QAbstractItemModel {
public:
  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override {
    return createIndex(row, column, nullptr); // 使用createIndex工厂方法
  }

  QModelIndex parent(const QModelIndex&) const override { return {}; }
  int rowCount(const QModelIndex&) const override { return 0; }
  int columnCount(const QModelIndex&) const override { return 0; }
  QVariant data(const QModelIndex&, int) const override { return {}; }
};

class SelectionsRangeHelperTest : public MyTestSuite {
  Q_OBJECT
public:
  SelectionsRangeHelperTest() : MyTestSuite{false} {}
private slots:
  void test_1_index_clear_ok() {
    SelectionsRangeHelper rngHelper;
    TestModel model;
    QModelIndex ind00 = model.index(0, 0);

    rngHelper.Set("/home/to/path", {ind00});
    QCOMPARE(rngHelper.currentPath, "/home/to/path");
    QCOMPARE(rngHelper.mRowRangeList, (QList<std::pair<int, int>>{{0,0}}));
    QVERIFY(rngHelper.mSelectedRowBits.any());
    QVERIFY(rngHelper.contains("/home/to/path", 0));
    QVERIFY(!rngHelper.contains("/home/to/path", 1));

    rngHelper.clear();
    QVERIFY(rngHelper.currentPath.isEmpty());
    QVERIFY(rngHelper.mRowRangeList.isEmpty());
    QVERIFY(rngHelper.mSelectedRowBits.none());

    QModelIndex ind10 = model.index(1, 0);
    rngHelper.Set("/home/to/path", {ind10});
    QVERIFY(!rngHelper.contains("/home/to/path", 0));
    QVERIFY(rngHelper.contains("/home/to/path", 1));
  }

  void test_all_index_ok() {
    SelectionsRangeHelper rngHelper;
    TestModel model;
    QModelIndex ind00 = model.index(0, 0);
    QModelIndex ind10 = model.index(1, 0);
    QModelIndex ind20 = model.index(2, 0);
    QModelIndex ind30 = model.index(3, 0);

    rngHelper.Set("/home/to/path", {ind00, ind10, ind20, ind30});
    QCOMPARE(rngHelper.currentPath, "/home/to/path");
    QCOMPARE(rngHelper.mRowRangeList, (QList<std::pair<int, int>>{{0,3}}));
    QCOMPARE(rngHelper.GetTopBottomRange().size(), 1);
    QVERIFY(rngHelper.mSelectedRowBits.any());
    QVERIFY(rngHelper.contains("/home/to/path", 0));
    QVERIFY(rngHelper.contains("/home/to/path", 1));
    QVERIFY(rngHelper.contains("/home/to/path", 2));
    QVERIFY(rngHelper.contains("/home/to/path", 3));
    QVERIFY(!rngHelper.contains("/home/to/path", 4096));
    QVERIFY(!rngHelper.contains("/home/to/path", 4097));
  }

  void test_discete_index_ok() {
    SelectionsRangeHelper rngHelper;
    TestModel model;
    QModelIndex ind00 = model.index(0, 0);
    QModelIndex ind10 = model.index(1, 0);

    QModelIndex ind30 = model.index(3, 0);

    QModelIndex ind50 = model.index(5, 0);
    QModelIndex ind60 = model.index(6, 0);

    rngHelper.Set("/home/to/path", {ind00, ind10, ind30, ind50, ind60});
    QCOMPARE(rngHelper.currentPath, "/home/to/path");
    QCOMPARE(rngHelper.mRowRangeList, (QList<std::pair<int, int>>{{0,1}, {3,3}, {5,6}}));
    QCOMPARE(rngHelper.GetTopBottomRange().size(), 3);
    QVERIFY(rngHelper.mSelectedRowBits.any());

    QVERIFY(rngHelper.contains("/home/to/path", 0));
    QVERIFY(rngHelper.contains("/home/to/path", 6));
    QVERIFY(!rngHelper.contains("path changed", 0));
    QVERIFY(!rngHelper.contains("path changed", 6));
  }
};

SelectionsRangeHelperTest g_SelectionsRangeHelperTest;
#include "SelectionsRangeHelperTest.moc"
