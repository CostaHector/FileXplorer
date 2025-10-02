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
using namespace MouseKeyboardEventHelper;

class ThumbnailImageViewerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME = "ThumbnailImageViewer";

  static QByteArray GetPNGImage(int width = 100, int height = 80, const QString& imgType = "PNG") {
    QByteArray imageData;
    QImage img(width, height, QImage::Format_RGB32);
    img.fill(Qt::green);
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, imgType.toUtf8().constData());
    return imageData;
  }

 private slots:
  void default_constructor_ok() {  //
    // should not crash down
    Configuration().setValue(THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME + "_ICON_SIZE_INDEX", 1);
    ThumbnailImageViewer viewer(THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME);
    QCOMPARE(viewer.m_memoryKeyName, THUMBNAIL_IMAGE_VIEWER_MEMORY_NAME);

    // 验证默认值
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    const QPixmap emptyPixmap = viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue);
    QVERIFY(emptyPixmap.isNull());
    QVERIFY(viewer.m_curDataByteArray.isEmpty());

    // 验证尺寸初始值
    QCOMPARE(viewer.mWidth, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].width());
    QCOMPARE(viewer.mHeight, IMAGE_SIZE::ICON_SIZE_CANDIDATES[1].height());

    viewer.setPixmapByByteArrayData("");  // emptyByteData
    QVERIFY(viewer.m_curDataByteArray.isEmpty());
    viewer.refreshPixmapSize();

    viewer.setIconSizeScaledIndex(-1);  // invalid value should not work
    QCOMPARE(viewer.GetCurImageSizeScale(), 1);

    viewer.clearPixmap();
    viewer.wheelEvent(nullptr);
  }

  void test_setPixmapByByteArrayData() {
    ThumbnailImageViewer viewer("testKey");

    QByteArray imageData = GetPNGImage(800, 600, "PNG");
    {
      viewer.mWidth = 8;  // modify to test scaledByWidth
      viewer.mHeight = 100;
      viewer.setPixmapByByteArrayData(imageData);
      QCOMPARE(viewer.m_curDataByteArray, imageData);
      QPixmap pm = viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue);
      // actual: 4:3 ==> max S = 8*6=48
      // avail: 8:100
      QCOMPARE(pm.width() * pm.height(), 8 * 6);
    }

    {
      viewer.mWidth = 100;  // modify to test scaledByHeight
      viewer.mHeight = 9;
      viewer.setPixmapByByteArrayData(imageData);
      QCOMPARE(viewer.m_curDataByteArray, imageData);
      QPixmap pm = viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue);
      // actual: 4:3 ==> max S = (4*3) * (3*3)=12*9=108
      // avail: 100:9
      QCOMPARE(pm.width() * pm.height(), 12 * 9);
    }
  }

  void test_wheelEvent_zoom() {
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

  void test_refreshPixmapSize() {
    ThumbnailImageViewer viewer("testKey");
    viewer.setIconSizeScaledIndex(4);
    // 创建测试图片
    QByteArray imageData = GetPNGImage(800, 600, "PNG");

    // 设置图片
    viewer.setPixmapByByteArrayData(imageData);
    QSize originalSize = viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue).size();

    // 改变缩放级别
    viewer.setIconSizeScaledIndex(viewer.GetCurImageSizeScale() + 1);
    viewer.refreshPixmapSize();

    // 验证图片尺寸已更新
    QSize newSize = viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue).size();
    QVERIFY(originalSize != newSize);
  }

  void test_clearPixmap() {
    ThumbnailImageViewer viewer("testKey");

    // 创建并设置图片
    QByteArray imageData = GetPNGImage(800, 600, "PNG");
    viewer.setPixmapByByteArrayData(imageData);

    // 清除图片
    viewer.clearPixmap();

    // 验证图片已清除
    QVERIFY(viewer.pixmap(Qt::ReturnByValueConstant::ReturnByValue).isNull());
    QVERIFY(viewer.m_curDataByteArray.isEmpty());
  }

  void test_destructor_save_load() {
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
