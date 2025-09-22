#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "LeftVideoGroupsModel.h"
#include "EndToExposePrivateMember.h"

class LeftVideoGroupsModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initalized_ok() {
    // with configs
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, 1 * 1000);  // 1 s
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, 1 * 1024);  // 1 kB
    {
      LeftVideoGroupsModel model;
      QCOMPARE(model.rowCount(), 0);
      QCOMPARE(model.columnCount(), 2);
      QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);
      QCOMPARE(model.m_deviationDur, 1 * 1000);
      QCOMPARE(model.m_deviationSz, 1 * 1024);
      QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Count");
      QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "SIZE");
    }

    // with no config
    Configuration().clear();
    {
      LeftVideoGroupsModel model;
      QCOMPARE(model.m_deviationDur, 2 * 1000);
      QCOMPARE(model.m_deviationSz, 2 * 1024);
    }
  }

  void differTypeChangeOnly_ok() {
    LeftVideoGroupsModel model;
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);

    DupVidMetaInfoList testData = {
        //
        {"v1.mp4", 1024, 10000, 0, "/v1.mp4", "hash1", true},  //
        {"v2.mp4", 2048, 20000, 0, "/v2.mp4", "hash2", true},  //
    };                                                         //
    model.onDuplicateVideosListChanged(testData);
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "SIZE");

    model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::DURATION);

    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "DURATION");

    model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::SIZE);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);
  }

  void DeviationSizeChangeOnly_ok() {
    Configuration().clear();

    LeftVideoGroupsModel model;
    model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::SIZE);

    DupVidMetaInfoList testData = {
        {"v1.mp4", 1000, 1000, 0, "/v1.mp4", "hash1", true},  //
        {"v2.mp4", 1000, 1000, 0, "/v2.mp4", "hash2", true},  //
        {"v3.mp4", 1001, 1000, 0, "/v3.mp4", "hash3", true},  //
        {"v4.mp4", 2000, 2000, 0, "/v4.mp4", "hash4", true},  //
        {"v5.mp4", 2000, 2000, 0, "/v5.mp4", "hash5", true},  //
    };

    model.onDuplicateVideosListChanged(testData);
    QCOMPARE(model.rowCount(), 2);

    model.setDeviationSize(500);
    QCOMPARE(model.rowCount(), 2);

    // deviation large enough round(2000/4001) = round(0.499) = 0
    model.setDeviationSize(4001);
    QCOMPARE(model.rowCount(), 1);
  }

  void deviationDurationChange_only_ok() {
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, 1050);  // 1050ms

    LeftVideoGroupsModel model;
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 0);
    QCOMPARE(model.rowCount(), 0);

    DupVidMetaInfoList testData = {
        {"v1.mp4", 1000, 1000, 0, "/v1.mp4", "hash1", true},  //
        {"v2.mp4", 1000, 1000, 0, "/v2.mp4", "hash2", true},  //
        {"v3.mp4", 1000, 1001, 0, "/v3.mp4", "hash3", true},  //
        {"v4.mp4", 1000, 2000, 0, "/v4.mp4", "hash4", true},  //
        {"v5.mp4", 1000, 2001, 0, "/v5.mp4", "hash5", true},  //
    };
    // [1000, 1000, 1001]/1050=1, [2000, 2001]/1050=2
    QCOMPARE(model.onDuplicateVideosListChanged(testData), 2 - 0);
    QCOMPARE(model.rowCount(), 2);

    // [1000, 1000]/1=1000, [1001]/1=1001, [2000]/1=2000, [2001]/1=2001
    model.setDeviationDuration(1);
    QCOMPARE(model.rowCount(), 1);

    // deviation large enough round(2001/4003) = round(0.499) = 0
    model.setDeviationDuration(4003);
    QCOMPARE(model.rowCount(), 1);
  }

  void dataUpdate_increase_decrease_row_count_ok() {
    Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, 2048);  // 2 kB

    LeftVideoGroupsModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);

    // 1th
    DupVidMetaInfoList testData1 = {
        {"v1.mp4", 1024, 10000, 0, "/v1.mp4", "hash1", true},             //
        {"v1_dup.mp4", 1024, 10000, 0, "/v1_dup.mp4", "hash1_dup", true}  //
    };
    // [1024, 1024] round 2048=1
    model.onDuplicateVideosListChanged(testData1);
    QCOMPARE(model.rowCount(), 1);

    // 2nd
    DupVidMetaInfoList testData2 = {
        {"v1.mp4", 1024, 10000, 0, "/v1.mp4", "hash1", true},                 //
        {"v1_dup.mp4", 1024, 10000, 0, "/v1_dup.mp4", "hash1_dup", true},     //
        {"v2.mp4", 4096 - 1, 20000, 0, "/v2.mp4", "hash2", true},             //
        {"v2_dup.mp4", 4096 - 1, 20000, 0, "/v2_dup.mp4", "hash2_dup", true}  //
    };
    // [1024, 1024] round 2048=1, [4096-1, 4096-1] round 2048=2
    model.onDuplicateVideosListChanged(testData2);
    QCOMPARE(model.rowCount(), 2);

    // 3rd
    DupVidMetaInfoList testData3 = {
        //
        {"v2.mp4", 2048, 20000, 0, "/v2.mp4", "hash2", true},             //
        {"v2_dup.mp4", 2048, 20000, 0, "/v2_dup.mp4", "hash2_dup", true}  //
    };
    model.onDuplicateVideosListChanged(testData3);
    QCOMPARE(model.rowCount(), 1);

    // 4th
    model.onDuplicateVideosListChanged({});
    QCOMPARE(model.rowCount(), 0);
  }

  void mismatch_devUnchange_willNotModifyGrouped_ok() {
    LeftVideoGroupsModel model;
    QCOMPARE(model.m_groupedVidLstArrChangedTimesInTestCase, 0);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::SIZE), 0);
    QCOMPARE(model.setDeviationDuration(model.m_deviationDur), 0);
    QCOMPARE(model.setDeviationSize(model.m_deviationSz), 0);
    QCOMPARE(model.m_groupedVidLstArrChangedTimesInTestCase, 0);

    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 0); // here modify grouped list once
    QCOMPARE(model.m_groupedVidLstArrChangedTimesInTestCase, 1);
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 0);
    QCOMPARE(model.setDeviationSize(model.m_deviationSz), 0);
    QCOMPARE(model.setDeviationDuration(model.m_deviationDur), 0);
    QCOMPARE(model.m_groupedVidLstArrChangedTimesInTestCase, 1);
  }

  void dataUpdate_and_differ_type_and_deviation_ok() {
    Configuration().clear();

    LeftVideoGroupsModel model;
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);
    QCOMPARE(model.m_deviationDur, 2 * 1000);  // 2s
    QCOMPARE(model.m_deviationSz, 2 * 1024);   // 2kB

    // name, sz, dur, date, abspath, hash, exists
    // sz rounding group:  [1k, 1k, 2k, 2k]/2048=1, [3k]/2048=2 => only 1 group
    // dur rounding group: [10s, 10s]/2s=2, [20s, 20s]/2s=10, [30s]/2s=15 => 2 groups
    DupVidMetaInfoList testData = {
        {"video1.mp4", 1024, 10000, 0, "/path/video1.mp4", "hash1", true},  //
        {"video2.mp4", 1024, 10000, 0, "/path/video2.mp4", "hash2", true},  //
        {"video3.mp4", 2048, 20000, 0, "/path/video3.mp4", "hash3", true},  //
        {"video4.mp4", 2048, 20000, 0, "/path/video4.mp4", "hash4", true},  //
        {"video5.mp4", 3072, 30000, 0, "/path/video5.mp4", "hash5", true},  //
    };
    // 1 by default size group
    QCOMPARE(model.onDuplicateVideosListChanged(testData), 1);
    QCOMPARE(model.rowCount(), 1);  // now row=1

    QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE].size(), 1);
    QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::DURATION].size(), 2);
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "SIZE");
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 4);                          // count=4, [1k, 1k, 2k, 2k]
    QCOMPARE(model.data(model.index(0, 1)).toInt(), 1024);                       // SIZE of first element
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::SIZE), 0);  // already group by size, skip
    decltype(model.m_groupedVidLstArr[0]) groupedVidLstBySizeBkp                 //
        = model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE];

    // 2 switch to duration group
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 2 - 1);  // now row=2, differ=2-1
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::DURATION);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "DURATION");

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 2);                              // count=2, [10s, 10s]
    QCOMPARE(model.data(model.index(0, 1)).toInt(), 10000);                          // DURATION of first element
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 0);  // already group by duration, skip
    decltype(model.m_groupedVidLstArr[0]) groupedVidLstByDurationBkp                 //
        = model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::DURATION];

    // 3 switch back to size group
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::SIZE), -1);  // now row=1, differ=1-2
    QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE], groupedVidLstBySizeBkp);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::SIZE);

    {
      // 4. Deviation Size changed/unchange behavior ok
      QCOMPARE(model.setDeviationDuration(60 * 1000), 0);  // current differ by mismatch duration, dev duration will not changed to 60s
      QVERIFY(model.m_deviationDur != 60 * 1000);
      QCOMPARE(model.setDeviationSize(model.m_deviationSz), 0);  // dev size unchange at all
      QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE], groupedVidLstBySizeBkp);

      // 4.1 deviation large enough all elements in one group
      // sz rounding group:  [1k, 1k, 2k, 2k, 3k]/10M=0 => 1 group
      QCOMPARE(model.setDeviationSize(10 * 1024 * 1024), 1 - 1);  // dev size change to 10MB, row = 1
      QCOMPARE(model.rowCount(), 1);
      QCOMPARE(model.data(model.index(0, 0)).toInt(), 5);     // count = 5
      QCOMPARE(model.data(model.index(0, 1)).toInt(), 1024);  // count = 5

      // 4.2 group with 3 elements ahead of group with 2 elements
      // sz rounding group:  [1k, 1k]/2.5k = 0, [2k, 2k, 3k]/2.5k=1 => 2 group
      QCOMPARE(model.setDeviationSize(2048 + 512), 2 - 1);  // dev size change to 1kB, row = 2
      QCOMPARE(model.rowCount(), 2);
      QCOMPARE(model.data(model.index(0, 0)).toInt(), 3);  // [2k, 2k, 3k] count = 2,
      QCOMPARE(model.data(model.index(0, 1)).toInt(), 2048);
      QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);  // [1k, 1k] count = 2
      QCOMPARE(model.data(model.index(1, 1)).toInt(), 1024);

      decltype(model.m_groupedVidLstArr[0]) beforeTwiceUpdate  //
          = model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE];
      // 4.3 data update twice ok
      QCOMPARE(model.onDuplicateVideosListChanged(testData), 0);
      QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::SIZE], beforeTwiceUpdate);
    }

    // 5 switch back to duration group
    // dur rounding group: [10s, 10s]/2s=2, [20s, 20s]/2s=10, [30s]/2s=15 => 2 groups
    QCOMPARE(model.setDifferType(RedundantVideoTool::DIFFER_BY_TYPE::DURATION), 2 - 2);  // rowCount = 2
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::DURATION], groupedVidLstByDurationBkp);
    QCOMPARE(model.m_currentDiffer, RedundantVideoTool::DIFFER_BY_TYPE::DURATION);
    {
      // 5.1 Deviation Duration changed and unchange ok
      QCOMPARE(model.setDeviationSize(1000 * 1024 * 1024), 0);  // current differ by mismatch size, dev size will not changed to 100MiB
      QVERIFY(model.m_deviationSz != 1000 * 1024 * 1024);
      QCOMPARE(model.setDeviationDuration(model.m_deviationDur), 0);  // dev duration unchange at all

      // 5.2 deviation large enough all elements in one group
      // duration rounding group:  [10s, 10s, 20s, 20s, 30s]/100s=1 => 1 groups
      QCOMPARE(model.setDeviationDuration(100 * 1000), 1 - 2);  // dev duration change to 100s, row = 1
      QCOMPARE(model.rowCount(), 1);
      QCOMPARE(model.data(model.index(0, 0)).toInt(), 5);  // count = 5
      QCOMPARE(model.data(model.index(0, 1)).toInt(), 10 * 1000);
      decltype(model.m_groupedVidLstArr[0]) beforeTwiceUpdate  //
          = model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::DURATION];

      // 5.3 data update twice ok
      QCOMPARE(model.onDuplicateVideosListChanged(testData), 0);  // rowCount = 1
      QCOMPARE(model.m_groupedVidLstArr[(int)RedundantVideoTool::DIFFER_BY_TYPE::DURATION], beforeTwiceUpdate);
    }

    // 6 data update to empty ok
    QCOMPARE(model.onDuplicateVideosListChanged({}), 0 - 1);  // rowCount = 1
    QCOMPARE(model.rowCount(), 0);
  }

  void groupingAlgorithms_ok() {
    DupVidMetaInfoList testData = {{"v1.mp4", 1000, 1000, 0, "/v1.mp4", "h1", true},
                                   {"v2.mp4", 1004, 1003, 0, "/v2.mp4", "h2", true},
                                   {"v3.mp4", 2000, 2000, 0, "/v3.mp4", "h3", true},
                                   {"v4.mp4", 2002, 2001, 0, "/v4.mp4", "h4", true},
                                   {"v5.mp4", 3000, 3000, 0, "/v5.mp4", "h5", true}};

    GroupedDupVidList groupBySizeLst = LeftVideoGroupsModel::getSizeLst(testData, 10);  // 10 byte
    GroupedDupVidList expectGroupedBySizeLst{
        DupVidMetaInfoList{
            {"v1.mp4", 1000, 1000, 0, "/v1.mp4", "h1", true},
            {"v2.mp4", 1004, 1003, 0, "/v2.mp4", "h2", true},
        },
        DupVidMetaInfoList{
            {"v3.mp4", 2000, 2000, 0, "/v3.mp4", "h3", true},
            {"v4.mp4", 2002, 2001, 0, "/v4.mp4", "h4", true},
        },
    };
    QCOMPARE(groupBySizeLst, expectGroupedBySizeLst);

    GroupedDupVidList groupByDurationLst = LeftVideoGroupsModel::getDurationsLst(testData, 10);  // 10ms
    GroupedDupVidList expectGroupedByDurationLst{DupVidMetaInfoList{
                                                     {"v1.mp4", 1000, 1000, 0, "/v1.mp4", "h1", true},
                                                     {"v2.mp4", 1004, 1003, 0, "/v2.mp4", "h2", true},
                                                 },
                                                 DupVidMetaInfoList{
                                                     {"v3.mp4", 2000, 2000, 0, "/v3.mp4", "h3", true},
                                                     {"v4.mp4", 2002, 2001, 0, "/v4.mp4", "h4", true},
                                                 }};
    QCOMPARE(groupByDurationLst, expectGroupedByDurationLst);
  }
};

#include "LeftVideoGroupsModelTest.moc"
REGISTER_TEST(LeftVideoGroupsModelTest, false)
