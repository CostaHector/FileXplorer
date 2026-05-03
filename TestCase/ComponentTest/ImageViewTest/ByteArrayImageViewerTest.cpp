#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ByteArrayImageViewer.h"
#include "EndToExposePrivateMember.h"

#include "FileTool.h"
#include "ImageTool.h"
#include "Configuration.h"

class ByteArrayImageViewerTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QString pngPath = TESTCASE_ROOT_PATH "/test/resolution_ratio.png";
  QByteArray pngBa;
  const QSize BEFORE_PNG_SIZE{24, 16};
  const QString BEFORE_PNG_SIZE_STR{"24x16"};
  const QSize AFTER_PNG_SIZE{50, 50};

  const QString gifPath = TESTCASE_ROOT_PATH "/test/sample_Blink_WhiteBlackRed.gif";
  QByteArray gifBa;
  const QSize BEFORE_GIF_SIZE{100, 100};
  const QString BEFORE_GIF_SIZE_STR{"100x100"};

private slots:
  void initTestCase() {
    QVERIFY(QFile::exists(":/image_test/resolution_ratio"));
    QVERIFY(QFile::exists(":/image_test/sample_Blink_WhiteBlackRed"));

    pngBa = FileTool::ByteArrayReader(":/image_test/resolution_ratio");
    gifBa = FileTool::ByteArrayReader(":/image_test/sample_Blink_WhiteBlackRed");

    QVERIFY(pngBa.size() > 0);
    QVERIFY(gifBa.size() > 0);

    QCOMPARE(ImageTool::GetImageDimensionPixel(pngPath), BEFORE_PNG_SIZE);
    QCOMPARE(ImageTool::GetImageDimensionPixel(gifPath), BEFORE_GIF_SIZE);
  }

  void attribute_ok() {
    int iconSizeIndex{-1};
    {
      ByteArrayImageViewer noParentViewer{"noParentByteArrayImageViewer"};
      QVERIFY(noParentViewer.testAttribute(Qt::WidgetAttribute::WA_DeleteOnClose));
      QVERIFY(noParentViewer.windowFlags().testFlag(Qt::Window));

      QCOMPARE(noParentViewer.GetName(), "noParentByteArrayImageViewer");

      iconSizeIndex = noParentViewer.mCurIconScaledSizeIndex;
      QVERIFY(iconSizeIndex > 0);
    }
    // in destructor saved icon index
    QCOMPARE(Configuration().value("noParentByteArrayImageViewer/ICON_SIZE_INDEX", -1), iconSizeIndex);

    {
      QWidget parentWid;
      ByteArrayImageViewer withParentViewer{"withParentByteArrayImageViewer", &parentWid};
      QVERIFY(!withParentViewer.testAttribute(Qt::WidgetAttribute::WA_DeleteOnClose));
    }
    Configuration().clear();
  }

  void setPixmapByByteArrayData_ok() { //
    ByteArrayImageViewer baViewer{"ByteArrayImageViewer"};
    QVERIFY(!baViewer.isCurImageGif());

    QVERIFY(!baViewer.setPixmapByByteArrayData(QByteArray{}, "jpg"));
    QVERIFY(!baViewer.isCurImageGif());

    {
      QVERIFY(baViewer.mLabel->pixmap(Qt::ReturnByValue).isNull());
      QVERIFY(baViewer.setPixmapByByteArrayData(pngBa, "png"));
      QVERIFY(!baViewer.isCurImageGif());
      QCOMPARE(baViewer.GetImageFileSize(), pngBa.size());
      QVERIFY(!baViewer.mLabel->pixmap(Qt::ReturnByValue).isNull());

      QString pngWinTitle = baViewer.windowTitle();
      QVERIFY(pngWinTitle.contains("Resolution"));
      QVERIFY(!pngWinTitle.contains("test/resolution_ratio.png")); // from bytearray. know no path info
      QVERIFY(pngWinTitle.contains(BEFORE_PNG_SIZE_STR));
    }

    {
      QVERIFY(baViewer.mLabel->movie() == nullptr);
      QVERIFY(baViewer.setPixmapByByteArrayData(gifBa, "gif"));
      QVERIFY(baViewer.isCurImageGif());
      QCOMPARE(baViewer.GetImageFileSize(), gifBa.size());
      QVERIFY(baViewer.mLabel->movie() != nullptr);

      QString gifWinTitle = baViewer.windowTitle();
      QVERIFY(gifWinTitle.contains("GIF, Frames"));
      QVERIFY(!gifWinTitle.contains("test/sample_Blink_WhiteBlackRed.gif")); // from bytearray. know no path info
      QVERIFY(gifWinTitle.contains(BEFORE_GIF_SIZE_STR));
    }
  }

  void refreshPixmapSize_ok() {
    QVERIFY(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N >= 2);

    ByteArrayImageViewer viewer("ByteArrayImageViewer_refreshPixmapSize_ok");
    viewer.setIconSizeScaledIndex(0);

    QVERIFY(viewer.setPixmapByByteArrayData(gifBa, "gif"));
    const QString winTitle1{viewer.windowTitle()};

    QVERIFY(viewer.setIconSizeScaledIndex(IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1));
    QVERIFY(viewer.refreshPixmapSize());
    const QString winTitle2{viewer.windowTitle()};
    QVERIFY(winTitle1 != winTitle2);

    QVERIFY(viewer.refreshPixmapSize()); // scale not change at all
    const QString winTitle22{viewer.windowTitle()};
    QCOMPARE(winTitle22, winTitle2);

    viewer.clearPixmap();
  }
};

#include "ByteArrayImageViewerTest.moc"
REGISTER_TEST(ByteArrayImageViewerTest, false)
