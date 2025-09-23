#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "RightVideoDuplicatesModel.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

void createTestData(GroupedDupVidListArr& data) {
  // SIZE
  GroupedDupVidList szGrp{
      DupVidMetaInfoList{
          // QString name; qint64 sz; int dur; qint64 modifiedDate; QString abspath; QString hash;
          {"sz - v1.mp4", 1000, 4000, 0, "notExist/sz - v1.mp4", ""},  //
          {"sz_v2.mp4", 1000, 4000, 0, __FILE__, "h2"},                  //
          {"sz_v3.mp4", 1000, 4000, 0, "/sz_v3.mp4", "h3"},              //
          {"sz_v4.mp4", 1000, 4000, 0, "/sz_v4.mp4", "h4"},              //
      },                                                                       //
      DupVidMetaInfoList{
          //
          {"sz_v5.mp4", 2000, 10000, 0, "/sz_v5.mp4", "h5"},  //
          {"sz_v6.mp4", 2000, 10000, 0, "/sz_v6.mp4", "h6"},  //
          {"sz_v7.mp4", 2000, 10000, 0, "/sz_v7.mp4", "h7"},  //
      },                                                            //
      DupVidMetaInfoList{
          //
          {"sz_v8.mp4", 4000, 55555, 0, "/sz_v8.mp4", "h8"},  //
          {"sz_v9.mp4", 4000, 55555, 0, "/sz_v9.mp4", "h9"},  //
      },                                                            //
  };
  data[(int)DuplicateVideoDetectionCriteria::DVCriteriaE::SIZE] = szGrp;
  // DURATION
  GroupedDupVidList durGrp{
      DupVidMetaInfoList{
          {"sz - v1.mp4", 1000, 4000, 0, "notExist/sz - v1.mp4", "h1"},  //
          {"sz_v2.mp4", 1000, 4000, 0, __FILE__, "h2"},                  //
          {"sz_v3.mp4", 1000, 4000, 0, "/sz_v3.mp4", "h3"},              //
          {"sz_v4.mp4", 1000, 4000, 0, "/sz_v4.mp4", "h4"},              //
      },                                                                       //
      DupVidMetaInfoList{
          //
          {"sz_v5.mp4", 2000, 10000, 0, "/sz_v5.mp4", "h5"},  //
          {"sz_v6.mp4", 2000, 10000, 0, "/sz_v6.mp4", "h6"},  //
          {"sz_v7.mp4", 2000, 10000, 0, "/sz_v7.mp4", "h7"},  //
      },                                                            //
  };
  data[(int)DuplicateVideoDetectionCriteria::DVCriteriaE::DURATION] = durGrp;
}

class RightVideoDuplicatesModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initialization_ok() {
    RightVideoDuplicatesModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), DuplicateVideoMetaInfo::DV_TABLE_HEADERS_COUNT);
    QCOMPARE(model.getLeftSelectedRow(), INVALID_LEFT_SELECTED_ROW);
    // here not bind these 2 memeber are nullptr
    QVERIFY(model._pGroupedVidsList == nullptr);
    QVERIFY(model._pCurrentDiffer == nullptr);

    for (int col = 0; col < DuplicateVideoMetaInfo::DV_TABLE_HEADERS_COUNT; ++col) {
      QCOMPARE(model.headerData(col, Qt::Horizontal, Qt::DisplayRole).toString(),  //
               DuplicateVideoMetaInfo::DV_TABLE_HEADERS[col]);
    }
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), ((int)Qt::AlignRight));

    QModelIndex invalidIndex;
    QVERIFY(model.data(invalidIndex).isNull()); // nullptr protected

    QCOMPARE(model.filePath(invalidIndex), "");
    QCOMPARE(model.fileNameUsedForToolEverything(invalidIndex), "");
  }

  void syncFrom_ok() {
    RightVideoDuplicatesModel model;

    GroupedDupVidListArr testData;
    createTestData(testData);
    DuplicateVideoDetectionCriteria::DVCriteriaE differ = DuplicateVideoDetectionCriteria::DVCriteriaE::SIZE;

    // when initialize, no row
    QCOMPARE(model.rowCount(), 0);
    model.SyncFrom(&testData, &differ);
    QCOMPARE(model._pGroupedVidsList, &testData);
    QCOMPARE(model._pCurrentDiffer, &differ);

    // when initialize, no row
    QCOMPARE(model.rowCount(), 0);

    // 1.0 differ by SIZE, each detail contains 4,3,2 elements respectively
    {
      model.onChangeDetailIndex(0);
      QCOMPARE(model.getLeftSelectedRow(), 0);
      QCOMPARE(model.rowCount(), 4);
      {  // 1.1 dataRetrieve o
        // {"Name", "Date", "Size", "Duration", "Hash", "FullPath"};
        QCOMPARE(model.data(QModelIndex{}).isNull(), true);

        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::Name)).toString(), "sz - v1.mp4");
        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::Size)).toString().isEmpty(), false);
        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::Duration)).toString().isEmpty(), false);
        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::ModifiedDate)).toDateTime().isNull(), false);
        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::Hash)).toString(), ""); // calculate md5 instantly, but file not exist
        QString notExistAbsFullPath = model.data(model.index(0, DuplicateVideoMetaInfo::AbsPath)).toString();
        QCOMPARE(model.filePath(model.index(0, DuplicateVideoMetaInfo::Name)), notExistAbsFullPath);
        QCOMPARE(model.fileNameUsedForToolEverything(model.index(0, DuplicateVideoMetaInfo::Name)), "sz   v1 mp4");  // only [0-9a-zA-Z_ ] in ans

        {  // line1 file not exist line color: gray
          QCOMPARE(QFile::exists(notExistAbsFullPath), false);
          QVariant inexistsRedForeground = model.data(model.index(0, DuplicateVideoMetaInfo::Name), Qt::ForegroundRole);
          QVERIFY(inexistsRedForeground.isValid());
          QVERIFY(inexistsRedForeground.canConvert<QBrush>());
          QBrush brush = inexistsRedForeground.value<QBrush>();
          QCOMPARE(brush.color(), QColor(Qt::gray));
        }

        QString existAbsFullPath = model.data(model.index(1, DuplicateVideoMetaInfo::AbsPath)).toString();
        QCOMPARE(existAbsFullPath.isEmpty(), false);
        {  // line2 fileexist line color: black
          QCOMPARE(QFile::exists(existAbsFullPath), true);
          QVariant inexistsRedForeground = model.data(model.index(1, DuplicateVideoMetaInfo::Name), Qt::ForegroundRole);
          QVERIFY(inexistsRedForeground.isValid());
          QVERIFY(inexistsRedForeground.canConvert<QBrush>());
          QBrush brush = inexistsRedForeground.value<QBrush>();
          QCOMPARE(brush.color(), QColor(Qt::black));
        }
        // Icon not null
        QCOMPARE(model.data(model.index(0, DuplicateVideoMetaInfo::Name), Qt::DecorationRole).isNull(), false);
        // out of bound protection;
        model.onChangeDetailIndex(999);
        QCOMPARE(model.data(QModelIndex{}, Qt::DecorationRole).isNull(), true);
      }

      model.onChangeDetailIndex(1);
      QCOMPARE(model.getLeftSelectedRow(), 1);
      QCOMPARE(model.rowCount(), 3);

      model.onChangeDetailIndex(2);
      QCOMPARE(model.getLeftSelectedRow(), 2);
      QCOMPARE(model.rowCount(), 2);

      model.onChangeDetailIndex(INVALID_LEFT_SELECTED_ROW);
      QCOMPARE(model.getLeftSelectedRow(), INVALID_LEFT_SELECTED_ROW);
      QCOMPARE(model.rowCount(), 0);

      model.onChangeDetailIndex(10);
      QCOMPARE(model.getLeftSelectedRow(), 10);
      QCOMPARE(model.rowCount(), 0);
    }

    differ = DuplicateVideoDetectionCriteria::DVCriteriaE::DURATION;
    model.onInvalidateLeftSelection();
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.isLeftSelectedRowValid(), false);

    // 2.0 differ by DURATION, each detail contains 4,3 elements respectively
    {
      model.onChangeDetailIndex(0);
      QCOMPARE(model.getLeftSelectedRow(), 0);
      QCOMPARE(model.rowCount(), 4);

      model.onChangeDetailIndex(1);
      QCOMPARE(model.getLeftSelectedRow(), 1);
      QCOMPARE(model.rowCount(), 3);

      model.onChangeDetailIndex(INVALID_LEFT_SELECTED_ROW);
      QCOMPARE(model.getLeftSelectedRow(), INVALID_LEFT_SELECTED_ROW);
      QCOMPARE(model.rowCount(), 0);

      model.onChangeDetailIndex(10);
      QCOMPARE(model.getLeftSelectedRow(), 10);
      QCOMPARE(model.rowCount(), 0);
    }
  }
};

#include "RightVideoDuplicatesModelTest.moc"
REGISTER_TEST(RightVideoDuplicatesModelTest, false)
