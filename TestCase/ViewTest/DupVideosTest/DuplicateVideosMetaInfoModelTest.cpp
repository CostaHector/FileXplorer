#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DuplicateVideosMetaInfoModel.h"
#include "EndToExposePrivateMember.h"

class DuplicateVideosMetaInfoModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initialization_and_update_ok() {
    DuplicateVideosMetaInfoModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 2);

    // horizontal header ok
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), QString("Table name"));
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), QString("Count"));
    {
      DupVidTableName2RecordCountList newData3{{"table 1", 20}, {"table 2", 30}, {"table 3", 40}};
      model.UpdateDupVideoTableMetaInfoList(newData3);
      QCOMPARE(newData3.size(), 0);  // get swapped
      QCOMPARE(model.rowCount(), 3);
      // vertical header: line number ok
      QCOMPARE(model.headerData(0, Qt::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1);
      QCOMPARE(model.headerData(1, Qt::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 2);
      QCOMPARE(model.headerData(2, Qt::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 3);

      QCOMPARE(model.headerData(0, Qt::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), ((int)Qt::AlignRight));
      QVERIFY(model.headerData(0, Qt::Horizontal, Qt::ItemDataRole::UserRole).isNull());
    }
    {
      DupVidTableName2RecordCountList newData2;
      newData2.append({"tableA", 5});
      newData2.append({"tableB", 15});

      model.UpdateDupVideoTableMetaInfoList(newData2);
      QCOMPARE(model.rowCount(), 2);
      QCOMPARE(model.data(model.index(0, 0)).toString(), QString("tableA"));
      QCOMPARE(model.data(model.index(1, 1)).toInt(), 15);
    }
    {
      DupVidTableName2RecordCountList newData0;
      model.UpdateDupVideoTableMetaInfoList(newData0);
      QCOMPARE(model.rowCount(), 0);
    }
  }

  void dataRetrieval_ok() {
    DupVidTableName2RecordCountList testData;
    testData.append({"table1", 10});
    testData.append({"table2", 20});
    testData.append({"table3", 30});

    DuplicateVideosMetaInfoModel model;
    model.UpdateDupVideoTableMetaInfoList(testData);
    QCOMPARE(model.rowCount(), 3);

    QModelIndex index0 = model.index(0, 0);
    QCOMPARE(model.data(index0).toString(), QString("table1"));
    QVERIFY(model.data(index0, Qt::ItemDataRole::UserRole).isNull());
    // return QIcon and the resource image not in test project
    QCOMPARE(model.data(index0, Qt::ItemDataRole::DecorationRole).isNull(), false);

    QModelIndex index1 = model.index(1, 1);
    QCOMPARE(model.data(index1).toInt(), 20);

    QModelIndex invalidIndex;
    QVERIFY(model.data(invalidIndex).isNull());
  }

  void fileNameRetrieval() {
    DupVidTableName2RecordCountList testData;
    testData.append({"table1", 10});
    testData.append({"table2", 20});

    DuplicateVideosMetaInfoModel model;
    model.UpdateDupVideoTableMetaInfoList(testData);

    QModelIndex validIndex = model.index(1, 0);
    QCOMPARE(model.fileName(validIndex), QString("table2"));

    QModelIndex invalidIndex;
    QCOMPARE(model.fileName(invalidIndex), QString());

    QModelIndex outOfRangeIndex = model.index(2, 0);
    QCOMPARE(model.fileName(outOfRangeIndex), QString());
  }

  void testFileNamesRetrieval() {
    DupVidTableName2RecordCountList testData;
    testData.append({"table1", 10});
    testData.append({"table2", 20});
    testData.append({"table3", 30});

    DuplicateVideosMetaInfoModel model;
    model.UpdateDupVideoTableMetaInfoList(testData);

    QModelIndexList indexes;
    indexes.append(model.index(0, 0));  // table1
    indexes.append(model.index(2, 0));  // table3

    QStringList fileNames = model.fileNames(indexes);
    QCOMPARE(fileNames.size(), 2);
    QCOMPARE(fileNames[0], QString("table1"));
    QCOMPARE(fileNames[1], QString("table3"));

    indexes.append(QModelIndex());  // append an invalid index, return default construct file name
    fileNames = model.fileNames(indexes);
    QCOMPARE(fileNames.size(), 3);
  }

  void boundaryConditions_ok() {
    DuplicateVideosMetaInfoModel model;

    // empty model
    QModelIndex index = model.index(0, 0);
    QVERIFY(model.data(index).isNull());
    QCOMPARE(model.fileName(index), QString());

    // out of range
    QModelIndex invalidColumnIndex = model.index(0, 2);
    QVERIFY(model.data(invalidColumnIndex).isNull());

    DupVidTableName2RecordCountList testData;
    testData.append({"table1", 10});
    model.UpdateDupVideoTableMetaInfoList(testData);

    QModelIndex outOfRangeIndex = model.index(1, 0);
    QVERIFY(model.data(outOfRangeIndex).isNull());
    QCOMPARE(model.fileName(outOfRangeIndex), QString());
  }
};

#include "DuplicateVideosMetaInfoModelTest.moc"
REGISTER_TEST(DuplicateVideosMetaInfoModelTest, false)
