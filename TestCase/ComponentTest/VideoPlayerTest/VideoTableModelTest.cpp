#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "VideoTableModel.h"
#include "VideoTableView.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

#include <QSignalSpy>

class VideoTableModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QList<FsNodeEntry> mNodes{
      {"file2.mp4", false, "22"},                                //
      {"file1.mkv", false, "1"},                                 //
      {"file1.json", false, R"({"Name": "file1", "Rate": 9})"},  //
      {"file3.txt", false, "333"},                               //
      {"VIDEO_TS/file5.avi", false, "55556"},                    // special folder: too small ignored in NORMAL mode
      {"folder/file4.avi", false, "4444"},                       //
      {"videos/file7.avi", false, "7777777"},                    // special folder: too small ignored in NORMAL mode
      {"vids/file6.avi", false, "666666"},                       // special folder: too small ignored in NORMAL mode
  };
 private slots:
  void initTestCase() {  //
    QVERIFY(mDir.IsValid());
    QCOMPARE(mDir.createEntries(mNodes), 8);
  }

  void cleanupTestCase() {  //
    mDir.ClearAll();
  }

  void default_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.flags({}).testFlag(Qt::ItemFlag::ItemIsEditable), false);
    QCOMPARE(videoModel.data({}).isValid(), false);
    QCOMPARE(videoModel.setData({}, 10, Qt::EditRole), false);

    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), Qt::AlignRight);
    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
             VideoTableModel::VIDEO_VERTICAL_HEAD[0]);
    QCOMPARE(videoModel.headerData(1, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
             VideoTableModel::VIDEO_VERTICAL_HEAD[1]);
    QCOMPARE(videoModel.headerData(99, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toInt(), 100);
    QCOMPARE(videoModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1);
    QCOMPARE(videoModel.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 2);

    QVERIFY(videoModel.rel2fileNames({}).isEmpty());
  }

  void setPlayPath_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QCOMPARE(videoModel.rootPath(), mWorkPath);
    QCOMPARE(videoModel.findMode(), VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY);
    // path unchange and not force mode, skip it
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY, false), 0);
    // path unchange and force mode
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY, true), 6);
    QCOMPARE(videoModel.forceReload(), 6);

    QCOMPARE(videoModel.setRootPath("inexists path"), 0);
    QCOMPARE(videoModel.rootPath(), "inexists path");
    QCOMPARE(videoModel.rowCount(), 0);

    // file inside special folder with size less then 10MiB will ignored
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::NORMAL), 2);
    QCOMPARE(videoModel.rowCount(), 2);
    QCOMPARE(videoModel.rootPath(), mWorkPath);
    QCOMPARE(videoModel.findMode(), VideoTableModel::VideoFindMode::NORMAL);
    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::FILE_NAME), Qt::DisplayRole).toString(), "file1.mkv");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::FILE_NAME), Qt::DisplayRole).toString(), "file2.mp4");
  }

  void data_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QCOMPARE(videoModel.data({}, Qt::DisplayRole), (QVariant{}));
    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::FILE_NAME), Qt::BackgroundRole), (QVariant{}));
    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::FILE_NAME), Qt::DisplayRole).toString(), "file1.mkv");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::FILE_NAME), Qt::DisplayRole).toString(), "file2.mp4");
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::FILE_NAME), Qt::DisplayRole).toString(), "file5.avi");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::REL_PATH), Qt::DisplayRole).toString(), "/");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::REL_PATH), Qt::DisplayRole).toString(), "/");
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::REL_PATH), Qt::DisplayRole).toString(), "/VIDEO_TS/");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::FILE_SIZE), Qt::DisplayRole).toString(), "0'0'0'1");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::FILE_SIZE), Qt::DisplayRole).toString(), "0'0'0'2");
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::FILE_SIZE), Qt::DisplayRole).toString(), "0'0'0'5");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::DURATION_FIELD), Qt::DisplayRole).toString(), "00:00:00");

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 9);
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 0);
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::SCORE_FIELD), Qt::DisplayRole).toInt(), 0);

    QCOMPARE(videoModel.data(videoModel.index(0, VideoBasicInfo::SCORE_FIELD), Qt::EditRole).toInt(), 9);
    QCOMPARE(videoModel.data(videoModel.index(1, VideoBasicInfo::SCORE_FIELD), Qt::EditRole).toInt(), 0);
    QCOMPARE(videoModel.data(videoModel.index(2, VideoBasicInfo::SCORE_FIELD), Qt::EditRole).toInt(), 0);
  }

  void updateDurationFields_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QSignalSpy dataChangedSpy{&videoModel, &VideoTableModel::dataChanged};

    QCOMPARE(videoModel.updateDurationFields({}), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    QModelIndexList indexes;
    indexes.reserve(2);
    indexes.push_back(videoModel.index(1, VideoBasicInfo::FILE_NAME));
    indexes.push_back(videoModel.index(3, VideoBasicInfo::FILE_NAME));
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

  void rateSelectedMovies_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);

    const QModelIndex firstIndex{videoModel.index(0, VideoBasicInfo::FILE_NAME)};
    const QModelIndex secondIndex{videoModel.index(1, VideoBasicInfo::FILE_NAME)};  // no json correspond
    const QModelIndexList indexes{firstIndex, secondIndex};

    QCOMPARE(firstIndex.data(Qt::DisplayRole).toString(), "file1.mkv");
    QCOMPARE(firstIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 9);
    QCOMPARE(secondIndex.data(Qt::DisplayRole).toString(), "file2.mp4");
    QCOMPARE(secondIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 0);  // no json correspond

    QSignalSpy dataChangedSpy{&videoModel, &VideoTableModel::dataChanged};
    QCOMPARE(videoModel.rateSelectedMovies({}, 10), 0);
    QCOMPARE(videoModel.rateSelectedMovies(indexes, 10), 1);  // 1st time

    QCOMPARE(firstIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 10);
    QCOMPARE(secondIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 0);  // no json correspond
    QCOMPARE(dataChangedSpy.count(), 1);
    dataChangedSpy.takeLast();

    QCOMPARE(videoModel.adjustRateSelectedMovies({}, 10), 0);
    QCOMPARE(videoModel.adjustRateSelectedMovies(indexes, 0), 0);  // delta = 0, skip

    QCOMPARE(videoModel.adjustRateSelectedMovies(indexes, -2), 1);
    QCOMPARE(firstIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 8);  // 1st time: 10-2
    QCOMPARE(videoModel.adjustRateSelectedMovies(indexes, 2), 1);
    QCOMPARE(firstIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 10);  // 2nd time: 10-2+2
    QCOMPARE(secondIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD).data(Qt::DisplayRole).toInt(), 0);  // no json correspond
    QCOMPARE(dataChangedSpy.count(), 2);
    dataChangedSpy.clear();
  }

  void setData_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);

    const QModelIndex firstIndex{videoModel.index(0, VideoBasicInfo::FILE_NAME)};
    const QModelIndex firstEditIndex{firstIndex.siblingAtColumn(VideoBasicInfo::SCORE_FIELD)};
    QCOMPARE(videoModel.flags(firstIndex).testFlag(Qt::ItemFlag::ItemIsEditable), false);
    QCOMPARE(videoModel.flags(firstEditIndex).testFlag(Qt::ItemFlag::ItemIsEditable), true);
    const int beforeRate = firstEditIndex.data().toInt();
    QCOMPARE(beforeRate, 10);
    const int newRate{9};

    // setData->rateSelectedMovies->emit dataChanged(index, index, {displayRole});
    QSignalSpy dataChangedSpy{&videoModel, &VideoTableModel::dataChanged};

    QCOMPARE(videoModel.setData(firstIndex, newRate, Qt::EditRole), false);           // column not accept
    QCOMPARE(videoModel.setData(firstEditIndex, newRate, Qt::DisplayRole), false);    // role not accept
    QCOMPARE(videoModel.setData(firstEditIndex, "not number", Qt::EditRole), false);  // value not accept

    QCOMPARE(videoModel.setData(firstEditIndex, beforeRate, Qt::EditRole), false);  // unchange not accept
    QCOMPARE(videoModel.setData(firstEditIndex, newRate, Qt::EditRole), true);      // changed ok
    QCOMPARE(videoModel.setData(firstEditIndex, beforeRate, Qt::EditRole), true);   // changed ok

    QCOMPARE(dataChangedSpy.count(), 2);
    dataChangedSpy.clear();
  }

  void tableView_setPlayPath_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(VideoBasicInfo::FILE_NAME, Qt::AscendingOrder);

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

  void AfterVideoFilesNameRenamed_ok() {
    VideoTableModel videoModel;
    QCOMPARE(videoModel.setRootPath(mWorkPath, VideoTableModel::VideoFindMode::INCLUDING_SUBDIRECTORY), 6);
    QCOMPARE(videoModel.rowCount(), 6);
    QModelIndexList lst{
        videoModel.index(0, VideoBasicInfo::FILE_NAME),
        videoModel.index(1, VideoBasicInfo::FILE_NAME),
        videoModel.index(3, VideoBasicInfo::FILE_NAME),
        videoModel.index(4, VideoBasicInfo::FILE_NAME),
    };
    QCOMPARE(videoModel.AfterVideoFilesNameRenamed(lst), 4);
    QCOMPARE(videoModel.rowCount(), 2);
  }
};

#include "VideoTableModelTest.moc"
REGISTER_TEST(VideoTableModelTest, false)
