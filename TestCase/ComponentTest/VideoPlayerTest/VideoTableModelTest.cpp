#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "VideoTableModel.h"
#include "VideoTableView.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class VideoTableModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QList<FsNodeEntry> mNodes{
      {"file2.mp4", false, "22"},              //
      {"file1.mkv", false, "1"},               //
      {"file3.txt", false, "333"},             //
      {"VIDEO_TS/file5.avi", false, "55556"},  // special folder: too small ignored in NORMAL mode
      {"folder/file4.avi", false, "4444"},     //
      {"videos/file7.avi", false, "7777777"},  // special folder: too small ignored in NORMAL mode
      {"vids/file6.avi", false, "666666"},     // special folder: too small ignored in NORMAL mode
  };
 private slots:
  void initTestCase() {  //
    QVERIFY(mDir.IsValid());
    QCOMPARE(mDir.createEntries(mNodes), 7);
  }

  void cleanupTestCase() {  //
    mDir.ClearAll();
  }

  void setPlayPath_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setPlayPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QCOMPARE(videoModel.GetPlayPath(), mWorkPath);

    QCOMPARE(videoModel.setPlayPath("inexists path"), 0);
    QCOMPARE(videoModel.GetPlayPath(), "inexists path");
    QCOMPARE(videoModel.rowCount(), 0);

    // file inside special folder with size less then 10MiB will ignored
    QCOMPARE(videoModel.setPlayPath(mWorkPath, VideoTableModel::VideoFindMode::NORMAL), 2);
    QCOMPARE(videoModel.rowCount(), 2);
    QCOMPARE(videoModel.GetPlayPath(), mWorkPath);
    QCOMPARE(videoModel.data(videoModel.index(0, 0), Qt::DisplayRole).toString(), "file1.mkv");
    QCOMPARE(videoModel.data(videoModel.index(1, 0), Qt::DisplayRole).toString(), "file2.mp4");

    // path unchange, skip it
    QCOMPARE(videoModel.setPlayPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 0);
  }

  void data_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), Qt::AlignRight);

    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
             VideoTableModel::VIDEO_VERTICAL_HEAD[0]);
    QCOMPARE(videoModel.headerData(1, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
             VideoTableModel::VIDEO_VERTICAL_HEAD[1]);
    QCOMPARE(videoModel.headerData(99, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toInt(), 100);
    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1);
    QCOMPARE(videoModel.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 2);

    QCOMPARE(videoModel.setPlayPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QCOMPARE(videoModel.data(videoModel.index(0, 0), Qt::DisplayRole).toString(), "file1.mkv");
    QCOMPARE(videoModel.data(videoModel.index(1, 0), Qt::DisplayRole).toString(), "file2.mp4");
    QCOMPARE(videoModel.data(videoModel.index(2, 0), Qt::DisplayRole).toString(), "file5.avi");

    QCOMPARE(videoModel.data(videoModel.index(0, 1), Qt::DisplayRole).toString(), "/");
    QCOMPARE(videoModel.data(videoModel.index(1, 1), Qt::DisplayRole).toString(), "/");
    QCOMPARE(videoModel.data(videoModel.index(2, 1), Qt::DisplayRole).toString(), "/VIDEO_TS/");

    QCOMPARE(videoModel.data(videoModel.index(0, 2), Qt::DisplayRole).toString(), "0'0'0'1");
    QCOMPARE(videoModel.data(videoModel.index(1, 2), Qt::DisplayRole).toString(), "0'0'0'2");
    QCOMPARE(videoModel.data(videoModel.index(2, 2), Qt::DisplayRole).toString(), "0'0'0'5");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 0);
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 0);
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 0);

    QVariant pixmapVar = videoModel.data(videoModel.index(0, VideoBasicInfo::SCORE_FIELD), Qt::DecorationRole);
    QVERIFY(pixmapVar.isValid());
    QVERIFY(pixmapVar.canConvert<QPixmap>());
    const QPixmap pixmapVarRate = pixmapVar.value<QPixmap>();
    QVERIFY(!pixmapVarRate.isNull());
  }

  void updateDurationFields_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setPlayPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QSignalSpy dataChangedSpy{&videoModel, &VideoTableModel::dataChanged};

    QCOMPARE(videoModel.updateDurationFields({}), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    QModelIndexList indexes;
    indexes.reserve(2);
    indexes.push_back(videoModel.index(1, 0));
    indexes.push_back(videoModel.index(3, 0));
    QCOMPARE(videoModel.updateDurationFields(indexes), 2);

    QModelIndex durationFieldBegInd{indexes.front().siblingAtColumn(VideoBasicInfo::DURATION_FIELD)};
    QModelIndex durationFieldEndInd{indexes.back().siblingAtColumn(VideoBasicInfo::DURATION_FIELD)};
    QVector<int> expectRoles{Qt::DisplayRole};

    QCOMPARE(dataChangedSpy.count(), 1);
    QVariantList parms = dataChangedSpy.takeLast();
    QModelIndex f = parms[0].value<QModelIndex>();
    QModelIndex e = parms[1].value<QModelIndex>();
    QVector<int> actualRoles = parms[2].value<QVector<int>>();
    QCOMPARE(f, durationFieldBegInd);
    QCOMPARE(e, durationFieldEndInd);
    QCOMPARE(actualRoles, expectRoles);

    // QCOMPARE(parms, (QVariantList{durationFieldBegInd, durationFieldEndInd, QVariant::fromValue(expectRoles)}));
    // don't do this, the last element differs
  }

  void tableView_setPlayPath_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QSignalSpy reqPlaySpy{&videoTv, &VideoTableView::reqPlayMedia};

    // 强制参数可携带
    videoTv.setPlayPath(mWorkPath, true);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 2);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{mDir.itemPath("file1.mkv"), true}));

    videoTv.setPlayPath("", true);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 0);
    QCOMPARE(reqPlaySpy.count(), 0);

    videoTv.setPlayPath(mWorkPath, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 2);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{mDir.itemPath("file1.mkv"), false}));
  }
};

#include "VideoTableModelTest.moc"
REGISTER_TEST(VideoTableModelTest, false)
