#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "ThumbnailImageViewer.h"
#include "EndToExposePrivateMember.h"
#include "StyleSheet.h"
#include "MouseKeyboardEventHelper.h"
#include "ImageTestPrecoditionTools.h"
#include "TDir.h"
#include "FileTool.h"
#include "StyleSheet.h"
#include <QScrollBar>
#include <QDesktopServices>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace MouseKeyboardEventHelper;
using namespace ImageTestPrecoditionTools;

class ThumbnailImageViewerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME = "ThumbnailImageViewer";
  TDir mTDir;
 private slots:
  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"a/img 1.jpg", false, GetPNGImage(100, 100, "jpg")},  //
        {"0/img 20.gif", false, ""},                           // invalid gif
        {"0/img 21.gif", false, GetTestGif()},                 // valid gif
        {"0/img 3.mp4", false, ""},                            //
        {"b/img 4.webp", false, GetPNGImage(40, 40, "webp")},  //
        {"img 6 broken.png", false, ""},                       //
        {"img 5.png", false, GetPNGImage(50, 50, "png")},      //
        {"img 7.png", false, GetPNGImage(70, 70, "png")},      //
    };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
  }

  void init() { GlobalMockObject::reset(); }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void default_constructor_ok() {  //
    // should not crash down
    Configuration().setValue(THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME + "_ICON_SIZE_INDEX", 1);
    ThumbnailImageViewer viewer(THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME);
    QCOMPARE(viewer.m_memoryKeyName, THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME);
    // 验证默认值
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    QVERIFY(viewer.mNavigateIntoSub != nullptr);
    QVERIFY(viewer.mMenu == nullptr);
    QVERIFY(viewer._OPEN_IN_SYSTEM_APPLICATION == nullptr);
    QVERIFY(viewer._REVEAL_IN_FILE_EXPLORER == nullptr);
    QVERIFY(viewer._COPY_FILE_NAME == nullptr);

    const QPixmap emptyPixmap = viewer.pixmap();
    QVERIFY(emptyPixmap.isNull());
    QVERIFY(viewer.mDataFromArchive.isEmpty());

    // 验证尺寸初始值
    QCOMPARE(viewer.mWidth, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].width());
    QCOMPARE(viewer.mHeight, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].height());

    viewer.setPixmapByByteArrayData("");  // emptyByteData
    QVERIFY(viewer.mDataFromArchive.isEmpty());
    viewer.refreshPixmapSize();

    viewer.setIconSizeScaledIndex(-1);  // invalid value should not work
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    viewer.clearPixmap();
    viewer.wheelEvent(nullptr);
    viewer.keyPressEvent(nullptr);
    viewer.showEvent(nullptr);
  }

  void setPixmap_local_image_file_ok() {
    ThumbnailImageViewer viewer{"testImagePath"};
    const ThumbnailImageViewer::FromPath expectImagea_1{mTDir.itemPath("a"), "img 1.jpg", mTDir.fileSize("a/img 1.jpg")};

    const ThumbnailImageViewer::FromPath expectImage0_20{mTDir.path(), "0/img 20.gif", mTDir.fileSize("0/img 20.gif")};
    const ThumbnailImageViewer::FromPath expectImage0_21{mTDir.path(), "0/img 21.gif", mTDir.fileSize("0/img 21.gif")};

    const ThumbnailImageViewer::FromPath expectImage5{mTDir.path(), "img 5.png", mTDir.fileSize("img 5.png")};
    const ThumbnailImageViewer::FromPath expectImage6{mTDir.path(), "img 6 broken.png", mTDir.fileSize("img 6 broken.png")};
    const ThumbnailImageViewer::FromPath expectImage7{mTDir.path(), "img 7.png", mTDir.fileSize("img 7.png")};

    {  // not including subdirectory
      // image: [5, 6, 7]
      QVERIFY(!viewer.mNavigateIntoSub->isChecked());

      QVERIFY(!viewer.setPixmapByAbsFilePath(mTDir.path(), "img 6 broken.png"));  // broken file
      QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::PATH);

      QVERIFY(viewer.setPixmapByAbsFilePath(mTDir.path(), "img 5.png"));  // valid file
      QCOMPARE(viewer.mDataFromPath, expectImage5);

      emit viewer.m_nextButton->clicked();
      QCOMPARE(viewer.mDataFromPath, expectImage6);

      QVERIFY(viewer.NavigateImageNext());  // 7 wrap to one
      QCOMPARE(viewer.mDataFromPath, expectImage7);

      QVERIFY(viewer.NavigateImageNext());
      QCOMPARE(viewer.mDataFromPath, expectImage5);

      emit viewer.m_prevButton->clicked();
      QCOMPARE(viewer.mDataFromPath, expectImage7);

      // now including subdirectory. 7 -> 0/img 20.gif
      viewer.mNavigateIntoSub->setChecked(true);
      emit viewer.mNavigateIntoSub->toggled(true);

      QVERIFY(!viewer.NavigateImageNext());  // 0/20, but this gif is broken
      QCOMPARE(viewer.mDataFromPath, expectImage0_20);

      QVERIFY(viewer.NavigateImageNext());  // 0/21, this gif is ok
      QCOMPARE(viewer.mDataFromPath, expectImage0_21);

      // to a folder contains only 1 image and has no subdirectory, no need navigate at all
      QVERIFY(viewer.setPixmapByAbsFilePath(mTDir.itemPath("a"), "img 1.jpg"));
      QVERIFY(viewer.NavigateImageNext());
      QCOMPARE(viewer.mDataFromPath, expectImagea_1);
      QVERIFY(viewer.NavigateImagePrevious());
      QCOMPARE(viewer.mDataFromPath, expectImagea_1);
    }
  }

  void file_path_text_copied_ok() {
    ThumbnailImageViewer viewer{"testImagePathPathCopied"};
    QVERIFY(viewer.setPixmapByAbsFilePath(mTDir.itemPath("a"), "img 1.jpg"));
    QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::PATH);

    const QString imgAbsPathCopied{mTDir.itemPath("a/img 1.jpg")};
    const QString imgAbsPathReveal{imgAbsPathCopied};
    const QUrl expectUrlOpened{QUrl::fromLocalFile(imgAbsPathCopied)};

    QVERIFY(viewer.mMenu == nullptr);
    viewer.onCustomContextMenuRequested(QPoint{0, 0});

    QVERIFY(viewer.mMenu != nullptr);
    QVERIFY(viewer._OPEN_IN_SYSTEM_APPLICATION != nullptr);
    QVERIFY(viewer._REVEAL_IN_FILE_EXPLORER != nullptr);
    QVERIFY(viewer._COPY_FILE_NAME != nullptr);

    MOCKER(QDesktopServices::openUrl).expects(exactly(1)).with(eq(expectUrlOpened)).will(returnValue(true));
    emit viewer._OPEN_IN_SYSTEM_APPLICATION->triggered();

    MOCKER(FileTool::RevealInSystemExplorer).expects(exactly(1)).with(eq(imgAbsPathReveal)).will(returnValue(true));
    emit viewer._REVEAL_IN_FILE_EXPLORER->triggered();

    MOCKER(FileTool::CopyTextToSystemClipboard).expects(exactly(1)).with(eq(imgAbsPathCopied)).will(returnValue(true));
    emit viewer._COPY_FILE_NAME->triggered();
  }

  void key_press_event_ok() {
    QString memoryName{"testImagePath_direction_key_scroll"};
    const int largestSize = 0;
    Configuration().value(memoryName + "_ICON_SIZE_INDEX", largestSize).toInt();

    ThumbnailImageViewer viewer{memoryName};
    QVERIFY(viewer.setPixmapByAbsFilePath(mTDir.itemPath("a"), "img 1.jpg"));
    QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::PATH);

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

    // 左上角
    hScroll->setValue(0);
    vScroll->setValue(0);
    int beforeHorValue{hScroll->value()}, beforeVerValue{vScroll->value()};
    QKeyEvent rightKey{QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier};
    viewer.keyPressEvent(&rightKey);
    QCOMPARE(rightKey.isAccepted(), true);
    QVERIFY(beforeHorValue < hScroll->value());

    QKeyEvent downKey{QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier};
    viewer.keyPressEvent(&downKey);
    QCOMPARE(downKey.isAccepted(), true);
    QVERIFY(beforeVerValue < vScroll->value());

    QKeyEvent leftKey{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier};
    viewer.keyPressEvent(&leftKey);
    QCOMPARE(leftKey.isAccepted(), true);
    QCOMPARE(hScroll->value(), 0);

    QKeyEvent upKey{QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier};
    viewer.keyPressEvent(&upKey);
    QCOMPARE(upKey.isAccepted(), true);
    QCOMPARE(vScroll->value(), 0);
  }

  void setPixmap_ByteArrayData_ok() {
    ThumbnailImageViewer viewer("testKey");

    QByteArray imageData = GetPNGImage(800, 600, "PNG");
    {
      viewer.mWidth = 8;  // modify to test scaledByWidth
      viewer.mHeight = 100;
      viewer.setPixmapByByteArrayData(imageData);
      QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::ARCHIVE);
      QCOMPARE(viewer.mDataFromArchive, imageData);
      QPixmap pm = viewer.pixmap();
      // actual: 4:3 ==> max S = 8*6=48
      // avail: 8:100
      QCOMPARE(pm.width() * pm.height(), 8 * 6);
    }

    {
      viewer.mWidth = 100;  // modify to test scaledByHeight
      viewer.mHeight = 9;
      viewer.setPixmapByByteArrayData(imageData);
      QCOMPARE(viewer.mDataFromArchive, imageData);
      QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::ARCHIVE);
      QPixmap pm = viewer.pixmap();
      // actual: 4:3 ==> max S = (4*3) * (3*3)=12*9=108
      // avail: 100:9
      QCOMPARE(pm.width() * pm.height(), 12 * 9);
    }
  }

  void wheelEvent_zoom_in_out_ok() {
    ThumbnailImageViewer viewer("testKey");

    viewer.setIconSizeScaledIndex(5);
    const QPoint downAngelDelta{0, 8 * 15};
    const QPoint upAngelDelta{0, -8 * 15};

    QSignalSpy spy(&viewer, &ThumbnailImageViewer::onImageScaledIndexChanged);
    {  // +1 accept
      QVERIFY(SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 6);
      QCOMPARE(spy.count(), 1);
      QCOMPARE(spy.back()[0].toInt(), 6);
    }

    {  // -1 accept
      QVERIFY(SendWheelEvent(viewer, upAngelDelta, Qt::KeyboardModifier::ControlModifier, true));
      QCOMPARE(viewer.GetCurImageSizeScale(), 5);
      QCOMPARE(spy.count(), 2);
      QCOMPARE(spy.back()[0].toInt(), 5);
    }

    {  // already 0, cannot -1. not accept
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

    {  // not with control modifier, will not change icon size, don't expect here
      SendWheelEvent(viewer, downAngelDelta, Qt::KeyboardModifier::NoModifier, true);
      QCOMPARE(viewer.GetCurImageSizeScale(), IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
      QCOMPARE(spy.count(), 2);
    }
  }

  void refreshPixmapSize_ok() {
    ThumbnailImageViewer viewer("testKey");
    viewer.setIconSizeScaledIndex(4);
    QByteArray imageData = GetPNGImage(800, 600, "PNG");
    viewer.setPixmapByByteArrayData(imageData);
    QSize originalSize = viewer.pixmap().size();
    viewer.setIconSizeScaledIndex(viewer.GetCurImageSizeScale() + 1);
    viewer.refreshPixmapSize();
    QSize newSize = viewer.pixmap().size();
    QVERIFY(originalSize != newSize);
  }

  void clearPixmap_ok() {
    ThumbnailImageViewer viewer("testKey");
    QVERIFY(!viewer.setPixmapByByteArrayData(""));  // invalid image
    QCOMPARE(viewer.mImageFrom, ThumbnailImageViewer::ImageFrom::ARCHIVE);

    QByteArray imageData = GetPNGImage(800, 600, "PNG");  // valid image
    QVERIFY(viewer.setPixmapByByteArrayData(imageData));

    QVERIFY(!viewer.NavigateImageNext());  // only support in from path
    QVERIFY(!viewer.NavigateImagePrevious());
    QVERIFY(!viewer.mImgIt.IsIncludingSubDirectory());
    QVERIFY(!viewer.NavigateIntoSubdirectoryChanged(true));
    QVERIFY(!viewer.mImgIt.IsIncludingSubDirectory());
    QVERIFY(!viewer.NavigateIntoSubdirectoryChanged(false));
    QVERIFY(!viewer.mImgIt.IsIncludingSubDirectory());

    viewer.clearPixmap();
    QVERIFY(viewer.pixmap().isNull());

    viewer.resizeEvent(nullptr);
    viewer.close();
  }

  void destructor_save_configuration_ok_load_ok() {
    const QString key = "testConfigKey";
    int testIndex = 7;
    {
      // 创建并设置缩放级别
      ThumbnailImageViewer viewer(key);
      viewer.setIconSizeScaledIndex(testIndex);
      // viewer析构时保存配置
    }

    // 验证配置已保存
    QCOMPARE(Configuration().value(key + "_ICON_SIZE_INDEX").toInt(), testIndex);
    {
      // 重新创建viewer，验证配置加载
      ThumbnailImageViewer viewer(key);
      QCOMPARE(viewer.GetCurImageSizeScale(), testIndex);
    }
  }
};

#include "ThumbnailImageViewerTest.moc"
REGISTER_TEST(ThumbnailImageViewerTest, false)
