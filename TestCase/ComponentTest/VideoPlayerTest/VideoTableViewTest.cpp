#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "VideoTableView.h"
#include "EndToExposePrivateMember.h"
#include "BatchRenameBy.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class VideoTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    VideoTableView videoTv;
    QVERIFY(videoTv.verticalHeader()->isHidden());
    QVERIFY(videoTv.mProxyModel != nullptr);
    QVERIFY(videoTv.mVideoModel != nullptr);
    QCOMPARE(videoTv.mPlaybackMode, QMediaPlaylist::PlaybackMode::CurrentItemOnce);
  }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void setMedias_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QSignalSpy reqPlaySpy{&videoTv, &VideoTableView::reqPlayMedia};

    QStringList inexistFiles{"CR7 0.mp4", "CR7 1.mp4", "CR7 2.mp4"};

    // 参数可携带
    videoTv.setMediaFiles("", inexistFiles, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 3);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", false}));

    // function double click to play here, 强制播放
    QModelIndex fIndex = videoTv.mProxyModel->index(0, VideoBasicInfo::FILE_NAME);
    QCOMPARE(fIndex.data(Qt::DisplayRole).toString(), "CR7 0.mp4");
    videoTv.setCurrentIndex(fIndex);
    videoTv.ReqPlay(fIndex, true);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));
  }

  void Iterator_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QSignalSpy reqPlaySpy{&videoTv, &VideoTableView::reqPlayMedia};

    QStringList inexistFiles{"CR7 0.mp4", "CR7 1.mp4", "CR7 2.mp4"};

    // 参数可携带
    videoTv.setMediaFiles("", inexistFiles, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 3);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", false}));
    QVERIFY(videoTv.selectionModel()->hasSelection());
    QVERIFY(videoTv.currentIndex().isValid());
    QCOMPARE(videoTv.currentIndex(), videoTv.mProxyModel->index(0, 0));

    // CurrentItemOnce
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemOnce);
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 0);

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 0);

    // CurrentItemInLoop 强制参数=true
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemInLoop);
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    // Sequential
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::Sequential);
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 0);

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 1.mp4", true}));

    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    // Loop 0->2, 2->0->1-2->2
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 2.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 1.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 2.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    // Random
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::Random);
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QVariantList parms1 = reqPlaySpy.takeLast();
    QCOMPARE(parms1.count(), 2);
    QCOMPARE(parms1.back(), true);  // 第一个参数随机， 第二个必定强制=true

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QVariantList parms2 = reqPlaySpy.takeLast();
    QCOMPARE(parms2.count(), 2);
    QCOMPARE(parms2.back(), true);

    // Sort Descending
    videoTv.mProxyModel->sort(0, Qt::DescendingOrder);  // CR7 2.mp4, CR7 1.mp4, CR7 0.mp4
    QModelIndex fIndex = videoTv.mProxyModel->index(0, VideoBasicInfo::FILE_NAME);
    QCOMPARE(fIndex.data(Qt::DisplayRole).toString(), "CR7 2.mp4");
    videoTv.setCurrentIndex(fIndex);

    // Loop
    videoTv.setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
    // 2 -> 0 -> 2 -> 1
    videoTv.PlayPreviousVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 0.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 2.mp4", true}));

    videoTv.PlayNextVideo();
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"CR7 1.mp4", true}));
  }

  void iterator_fail_no_current_row() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QSignalSpy reqPlaySpy{&videoTv, &VideoTableView::reqPlayMedia};
    QCOMPARE(videoTv.mVideoModel->rowCount(), 0);
    QVERIFY(!videoTv.iteratorCore(1).isValid());
    QVERIFY(!videoTv.iteratorCore(-1).isValid());

    QStringList inexistFiles{"Kaka 0.mp4", "Kaka 1.mp4", "Kaka 2.mp4"};
    videoTv.setMediaFiles("", inexistFiles, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 3);
    QCOMPARE(reqPlaySpy.count(), 1);
    QCOMPARE(reqPlaySpy.takeLast(), (QVariantList{"Kaka 0.mp4", false}));

    videoTv.clearSelection();
    videoTv.setCurrentIndex({});
    QVERIFY(!videoTv.currentIndex().isValid());
    QVERIFY(!videoTv.previousIndex().isValid());
    QVERIFY(!videoTv.nextIndex().isValid());

    emit videoTv.doubleClicked({});
  }

  void onRateSelectedMovies_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(videoTv.onRateSelectedMovies(10), 0);

    QStringList inexistFiles{"/Kaka 0.mp4", "/Kaka 1.mp4", "/Kaka 2.mp4"};
    videoTv.setMediaFiles("", inexistFiles, false);
    videoTv.clearSelection();
    QCOMPARE(videoTv.onRateSelectedMovies(10), 0);  // no selection

    QCOMPARE(videoTv.mProxyModel->rowCount(), 3);
    videoTv.selectAll();
    QCOMPARE(videoTv.onRateSelectedMovies(10), 0);  // no json at all
  }

  void onRenameJsonAndRelated_replace_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QStringList inexistFiles{"Kaka.mp4"};  // "Kaka.json", "Kaka 1.jpg", "Kaka 2.png"
    videoTv.setMediaFiles("/", inexistFiles, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 1);

    videoTv.clearSelection();
    QCOMPARE(videoTv.onRenameJsonAndRelatedReplace(), 0);  // no row selected

    videoTv.selectAll();
    MOCKER(BatchRenameBy::ReplaceBySpecifiedJson)
        .expects(exactly(2))    //
        .will(returnValue(0))   // 1st: user cancel, 0 files renamed, 0 row deleted
        .then(returnValue(3));  // 2nd: 3 files renamed, 1 row deleted
    QCOMPARE(videoTv.onRenameJsonAndRelatedReplace(), 0);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 1);

    QCOMPARE(videoTv.onRenameJsonAndRelatedReplace(), 3);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 0);  // 1 row deleted
  }

  void onRenameJsonAndRelated_Insert_ok() {
    VideoTableView videoTv;
    videoTv.mProxyModel->sort(0, Qt::AscendingOrder);

    QStringList inexistFiles{"Kaka.mp4"};  // "Kaka.json", "Kaka 1.jpg", "Kaka 2.png"
    videoTv.setMediaFiles("/", inexistFiles, false);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 1);

    videoTv.clearSelection();
    QCOMPARE(videoTv.onRenameJsonAndRelatedInsert(), 0);  // no row selected

    videoTv.selectAll();
    MOCKER(BatchRenameBy::InsertBySpecifiedJson)
        .expects(exactly(2))    //
        .will(returnValue(0))   // 1st: user cancel, 0 files renamed, 0 row deleted
        .then(returnValue(3));  // 2nd: 3 files renamed, 1 row deleted
    QCOMPARE(videoTv.onRenameJsonAndRelatedInsert(), 0);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 1);

    QCOMPARE(videoTv.onRenameJsonAndRelatedInsert(), 3);
    QCOMPARE(videoTv.mVideoModel->rowCount(), 0);  // 1 row deleted
  }
};

#include "VideoTableViewTest.moc"
REGISTER_TEST(VideoTableViewTest, true)
