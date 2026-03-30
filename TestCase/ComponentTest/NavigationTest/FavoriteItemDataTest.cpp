#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "FavoriteItemData.h"

class FavoriteItemDataTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:

  void read_write_ok() {
    FavoriteItemData defData;
    QCOMPARE(defData.isValid(), false);

    FavoriteItemData grpData{"group"};
    QCOMPARE(grpData.isValid(), true);

    FavoriteItemData nonGrpData{"non_group", "path/to/folder"};
    QCOMPARE(nonGrpData.isValid(), true);

    FavoriteItemData grpDataWithChild{"group_with_child"};
    grpDataWithChild.children.push_back(FavoriteItemData{"child_group_item"});
    grpDataWithChild.children.push_back(FavoriteItemData{"child_non_group_item", "child_path/to/folder"});

    QByteArray data;

    QDataStream dsWrite{&data, QIODevice::WriteOnly};
    dsWrite << defData << grpData << nonGrpData << grpDataWithChild;
    QCOMPARE(dsWrite.status(), QDataStream::Status::Ok);
    QCOMPARE(data.isEmpty(), false);

    FavoriteItemData defDataActual;
    FavoriteItemData grpDataActual;
    FavoriteItemData nonGrpDataActual;
    FavoriteItemData grpDataWithChildActual;

    QDataStream dsRead{data};
    dsRead >> defDataActual >> grpDataActual >> nonGrpDataActual >> grpDataWithChildActual;
    QCOMPARE(dsRead.status(), QDataStream::Status::Ok);
    QCOMPARE(data.isEmpty(), false);

    QCOMPARE(defDataActual, defData);
    QCOMPARE(grpDataActual, grpData);
    QCOMPARE(nonGrpDataActual, nonGrpData);
    QCOMPARE(grpDataWithChildActual, grpDataWithChild);
  }
};

#include "FavoriteItemDataTest.moc"
REGISTER_TEST(FavoriteItemDataTest, true)
