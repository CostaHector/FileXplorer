#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include <QSignalSpy>

#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "ThumbnailImageViewer.h"
#include "EndToExposePrivateMember.h"
#include "ImageTool.h"
#include "MouseKeyboardEventHelper.h"
#include "ImageTestPrecoditionTools.h"

#include "FileTool.h"
#include <QScrollBar>
#include <QDesktopServices>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace MouseKeyboardEventHelper;

class ThumbnailImageViewerTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QString parentPath = TESTCASE_ROOT_PATH "/test";

  const QString pngName = "resolution_ratio.png";
  const QString pngPath = parentPath + "/" + pngName;
  const QString BEFORE_PNG_SIZE_STR{"24x16"};

  const QString gifName = "sample_Blink_WhiteBlackRed.gif";
  const QString gifPath = parentPath + "/" + gifName;
  const QString BEFORE_GIF_SIZE_STR{"100x100"};
private slots:
  void initTestCase() { //
    // precondition
    const QDir parentDir{parentPath};
    QVERIFY(parentDir.exists(pngName));
    QVERIFY(parentDir.exists(gifName));

    Configuration().clear();
  }

  void init() { GlobalMockObject::reset(); }

  void cleanup() { GlobalMockObject::verify(); }

  void default_constructor_ok() { //
    // should not crash down
    IMAGE_SIZE::SaveInitialScaledSize("ThumbnailImageViewer_default_ok", 1);
    ThumbnailImageViewer viewer("ThumbnailImageViewer_default_ok");
    QCOMPARE(viewer.GetName(), "ThumbnailImageViewer_default_ok");
    // 验证默认值
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    QVERIFY(viewer.mNavigateIntoSub != nullptr);
    QVERIFY(viewer.mMenu == nullptr);
    QVERIFY(viewer._OPEN_IN_SYSTEM_APPLICATION == nullptr);
    QVERIFY(viewer._REVEAL_IN_FILE_EXPLORER == nullptr);
    QVERIFY(viewer._COPY_FILE_NAME == nullptr);

    // 验证尺寸初始值
    QCOMPARE(viewer.mWidth, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].width());
    QCOMPARE(viewer.mHeight, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].height());

    QVERIFY(!viewer.setPixmapByAbsFilePath("inexist file path.jpg", "jpg"));
    viewer.refreshPixmapSize();

    viewer.setIconSizeScaledIndex(-1); // invalid value should not work
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    viewer.clearPixmap();
    viewer.wheelEvent(nullptr);
    viewer.keyPressEvent(nullptr);
    viewer.showEvent(nullptr);

    QShowEvent sEvent;
    viewer.showEvent(&sEvent);
  }

  void setPixmapByAbsFilePath_ok() {
    ThumbnailImageViewer viewer{"ThumbnailImageViewer"};
    QVERIFY(!viewer.isCurImageGif());

    QVERIFY(!viewer.setPixmapByAbsFilePath(QByteArray{}, "jpg"));
    QVERIFY(!viewer.isCurImageGif());

    {
      QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, pngName));
      QVERIFY(!viewer.isCurImageGif());
      QString pngWinTitle = viewer.windowTitle();
      QVERIFY(pngWinTitle.contains("Resolution"));
      QVERIFY(pngWinTitle.contains(pngName)); // from local path. know no path info
      QVERIFY(pngWinTitle.contains(BEFORE_PNG_SIZE_STR));
    }

    {
      QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, gifName));
      QVERIFY(viewer.isCurImageGif());
      QString gifWinTitle = viewer.windowTitle();
      QVERIFY(gifWinTitle.contains("GIF, Frames"));
      QVERIFY(gifWinTitle.contains(gifName)); // from local path. know no path info
      QVERIFY(gifWinTitle.contains(BEFORE_GIF_SIZE_STR));
    }
  }

  void img_Navigate_ok() {
    ThumbnailImageViewer viewer{"ThumbnailImageViewer_Navigate_ok"};
    // not including subdirectory
    QVERIFY(!viewer.mNavigateIntoSub->isChecked());
    QVERIFY(!viewer.mImgIt.IsIncludingSubDirectory());

    QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, pngName));
    QCOMPARE(viewer.mRel2Image, pngName);

    // next, prev return to itself
    {
      viewer.m_nextButton->triggered();
      QVERIFY(viewer.mRel2Image != pngName);

      viewer.m_prevButton->triggered();
      QCOMPARE(viewer.mRel2Image, pngName);
    }

    QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, gifName));
    QCOMPARE(viewer.mRel2Image, gifName);

    // prev, next return to itself
    {
      QVERIFY(viewer.NavigateImagePrevious());
      QVERIFY(viewer.mRel2Image != gifName);

      QVERIFY(viewer.NavigateImageNext());
      QCOMPARE(viewer.mRel2Image, gifName);
    }

    viewer.mNavigateIntoSub->toggle();
    QVERIFY(viewer.mNavigateIntoSub->isChecked());
    QVERIFY(viewer.mImgIt.IsIncludingSubDirectory());

    viewer.NavigateIntoSubdirectoryChanged(false);
    QVERIFY(!viewer.mImgIt.IsIncludingSubDirectory());
  }

  void onCustomContextMenuRequested_ok() {
    ThumbnailImageViewer viewer{"ThumbnailImageViewer_menu_ok"};
    QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, gifName));

    QVERIFY(viewer.mMenu == nullptr);
    QVERIFY(viewer._OPEN_IN_SYSTEM_APPLICATION == nullptr);
    QVERIFY(viewer._REVEAL_IN_FILE_EXPLORER == nullptr);
    QVERIFY(viewer._COPY_FILE_NAME == nullptr);

    viewer.onCustomContextMenuRequested(QPoint{0, 0});

    QVERIFY(viewer.mMenu != nullptr);
    QVERIFY(viewer._OPEN_IN_SYSTEM_APPLICATION != nullptr);
    QVERIFY(viewer._REVEAL_IN_FILE_EXPLORER != nullptr);
    QVERIFY(viewer._COPY_FILE_NAME != nullptr);

    const QString imgAbsPathCopied{gifPath};
    const QString imgAbsPathReveal{gifPath};
    const QUrl expectUrlOpened{QUrl::fromLocalFile(gifPath)};

    MOCKER(QDesktopServices::openUrl).expects(exactly(1)).with(eq(expectUrlOpened)).will(returnValue(true));
    MOCKER(FileTool::RevealInSystemExplorer).expects(exactly(1)).with(eq(imgAbsPathReveal)).will(returnValue(true));
    MOCKER(FileTool::CopyTextToSystemClipboard).expects(exactly(1)).with(eq(imgAbsPathCopied)).will(returnValue(true));

    viewer._OPEN_IN_SYSTEM_APPLICATION->triggered();
    viewer._REVEAL_IN_FILE_EXPLORER->triggered();
    viewer._COPY_FILE_NAME->triggered();
  }

  void keyPressEvent_ok() {
    ThumbnailImageViewer viewer{"ThumbnailImageViewer_keyPressEvent_ok"};
    QVERIFY(viewer.setPixmapByAbsFilePath(parentPath, pngName));

    QKeyEvent ignoredKey{QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier};
    viewer.keyPressEvent(&ignoredKey);
    QCOMPARE(ignoredKey.isAccepted(), false);

    QScrollBar* hScroll = viewer.horizontalScrollBar();
    QScrollBar* vScroll = viewer.verticalScrollBar();
    QVERIFY(hScroll != nullptr);
    QVERIFY(vScroll != nullptr);
    hScroll->setMinimum(0);
    hScroll->setMaximum(100);
    vScroll->setMinimum(0);
    vScroll->setMaximum(100);

    // 初始在左上角
    hScroll->setValue(0);
    vScroll->setValue(0);

    // right direction ok
    int beforeHorValue{hScroll->value()}, beforeVerValue{vScroll->value()};
    QKeyEvent rightKey{QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier};
    viewer.keyPressEvent(&rightKey);
    QCOMPARE(rightKey.isAccepted(), true);
    QVERIFY(beforeHorValue < hScroll->value());

    // down direction ok
    QKeyEvent downKey{QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier};
    viewer.keyPressEvent(&downKey);
    QCOMPARE(downKey.isAccepted(), true);
    QVERIFY(beforeVerValue < vScroll->value());

    // left direction ok
    QKeyEvent leftKey{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier};
    viewer.keyPressEvent(&leftKey);
    QCOMPARE(leftKey.isAccepted(), true);
    QCOMPARE(hScroll->value(), 0);

    // up direction ok
    QKeyEvent upKey{QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier};
    viewer.keyPressEvent(&upKey);
    QCOMPARE(upKey.isAccepted(), true);
    QCOMPARE(vScroll->value(), 0);
  }

  void getDestSize_ok() {
    ThumbnailImageViewer viewer("ThumbnailImageViewer_getDestSize");
    QSize beforeSz{800, 600};
    {
      viewer.mWidth = 8; // modify to test scaledByWidth
      viewer.mHeight = 100;
      QSize expectAfterSz{8, 6};
      // actual: 4:3 ==> max S = 8*6=48
      // avail: 8:100
      QCOMPARE(viewer.getDestSize(beforeSz), expectAfterSz);
    }

    {
      viewer.mWidth = 100; // modify to test scaledByHeight
      viewer.mHeight = 9;
      QSize expectAfterSz{12, 9};
      // actual: 4:3 ==> max S = (4*3) * (3*3)=12*9=108
      // avail: 100:9
      QCOMPARE(viewer.getDestSize(beforeSz), expectAfterSz);
    }
  }

  void wheelEvent_zoom_in_out_ok() {
    ThumbnailImageViewer viewer("ThumbnailImageViewer_wheelEvent_ok");

    viewer.setIconSizeScaledIndex(5);
    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    QSignalSpy spy(&viewer, &ThumbnailImageViewer::onImageScaledIndexChanged);
    { // +1 accept
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 6);
      QCOMPARE(spy.count(), 1);
      QCOMPARE(spy.back()[0].toInt(), 6);
    }

    { // -1 accept
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 5);
      QCOMPARE(spy.count(), 2);
      QCOMPARE(spy.back()[0].toInt(), 5);
    }

    { // already 0, cannot -1. not accept
      viewer.setIconSizeScaledIndex(0);
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, false));
      QCOMPARE(viewer.GetCurImageSizeScale(), 0);
      QCOMPARE(spy.count(), 2);
    }

    {
      // already max, cannot +1. not accept
      viewer.setIconSizeScaledIndex(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, false));
      QCOMPARE(viewer.GetCurImageSizeScale(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }

    { // not with control modifier, will not change icon size, don't expect here
      SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::NoModifier, true);
      QCOMPARE(viewer.GetCurImageSizeScale(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }
  }
};

#include "ThumbnailImageViewerTest.moc"
REGISTER_TEST(ThumbnailImageViewerTest, false)
