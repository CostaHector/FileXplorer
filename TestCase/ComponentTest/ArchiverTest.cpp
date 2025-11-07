#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "Archiver.h"
#include "EndToExposePrivateMember.h"

#include "StyleSheet.h"
#include "DataFormatter.h"
#include "ImageTestPrecoditionTools.h"
using namespace ImageTestPrecoditionTools;

class ArchiverTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    Configuration().clear();  //
  }

  void default_constructor_ok() {
    Archiver archiver;
    // 验证核心组件初始化
    QVERIFY(archiver.m_splitter != nullptr);
    QVERIFY(archiver.m_itemsTable != nullptr);
    QVERIFY(archiver.m_archiverModel != nullptr);
    QVERIFY(archiver.m_thumbnailViewer != nullptr);
    QVERIFY(archiver.m_ImageSizeHint != nullptr);
    QVERIFY(archiver.m_ImageSizeScale != nullptr);

    // 验证初始状态
    QCOMPARE(archiver.windowTitle(), "QZ Archive");
    QCOMPARE(archiver.m_ImageSizeScale->value(), archiver.m_thumbnailViewer->GetCurImageSizeScale());
    archiver.close();
  }

  void test_operator_call_success() {
    Archiver archiver;
    // 模拟无效的QZ文件, 验证窗口标题更新
    QVERIFY(!archiver("path/inexist/to_not_qz.file"));
    QCOMPARE(archiver.windowTitle(), "ArchiveFilesPreview | [path/inexist/to_not_qz.file] not a qz file");

    QVERIFY(archiver("path/inexist/to_not_qz.qz"));
  }

  void test_onSelectNewItemRow_valid() {
    // precondition
    QByteArray validPngByteArray = GetPNGImage(80, 60, "PNG");
    const QString expectSizeInTitle = DataFormatter::formatFileSizeGMKB(validPngByteArray.size());

    Archiver archiver;
    QList<ArchiveFilesReader::CompressedItem> mockItems  //
        {
            {"file1.png", 2048, 9, validPngByteArray},
            {"file2.jpg", 4096, 10, QByteArray("Content  2")},
            {"folder/file3.pdf", 8192, 11, QByteArray("Content  3")},
        };
    archiver.m_archiverModel->RowsCountBeginChange(0, 3);
    archiver.m_archiverModel->m_paf.mockCompressedItem(mockItems);
    archiver.m_archiverModel->RowsCountEndChange();

    // 模拟选择第一行
    QModelIndex index = archiver.m_archiverModel->index(0, 0);
    bool result = archiver.onSelectNewItemRow(index, QModelIndex());
    QVERIFY(result);
    // 验证窗口标题更新
    const QString winTitle = archiver.windowTitle();
    QVERIFY(winTitle.contains("file1.png"));
    QVERIFY(winTitle.contains(expectSizeInTitle));  // 真实大小

    // 验证缩略图设置
    QVERIFY(archiver.m_thumbnailViewer != nullptr);
    QVERIFY(!archiver.m_thumbnailViewer->pixmap().isNull());
    {
      // 模拟无效选择, 验证缩略图清除
      result = archiver.onSelectNewItemRow(QModelIndex(), QModelIndex());
      QVERIFY(!result);
      QVERIFY(archiver.m_thumbnailViewer->pixmap().isNull());
    }

    // 模拟选择第二行（图片）
    index = archiver.m_archiverModel->index(1, 0);
    result = archiver.onSelectNewItemRow(index, QModelIndex());
    QVERIFY(result);

    // 验证窗口标题更新
    QVERIFY(archiver.windowTitle().contains("file2.jpg"));

    // 模拟选择第三行
    index = archiver.m_archiverModel->index(2, 0);
    result = archiver.onSelectNewItemRow(index, QModelIndex());
    QVERIFY(result);

    // 验证窗口标题更新
    QVERIFY(archiver.windowTitle().contains("file3.pdf"));
  }

  void test_slider_image_interaction() {
    QByteArray validPngByteArray = GetPNGImage(80, 60, "PNG");

    // precondition
    Configuration().clear();
    Archiver archiver;
    // silder value should not already be the biggest number
    QVERIFY(archiver.m_ImageSizeScale->value() < archiver.m_ImageSizeScale->maximum());

    // 准备模拟数据
    QList<ArchiveFilesReader::CompressedItem> mockItems  //
        {
            {"image.jpg", 2048, 1024, validPngByteArray},
        };

    // 设置模型数据
    archiver.m_archiverModel->RowsCountBeginChange(0, 1);
    archiver.m_archiverModel->m_paf.mockCompressedItem(mockItems);
    archiver.m_archiverModel->RowsCountEndChange();

    // 选择一行以加载图片
    QModelIndex index = archiver.m_archiverModel->index(0, 0);
    archiver.onSelectNewItemRow(index, QModelIndex());

    // 初始状态验证
    int initialScale = archiver.m_thumbnailViewer->GetCurImageSizeScale();
    QCOMPARE(archiver.m_ImageSizeScale->value(), initialScale);

    // 改变滑块值, 发射valueChanged, m_ImageSizeHint更新, m_thumbnailViewer更新
    QSignalSpy sliderSpy(archiver.m_ImageSizeScale, &QSlider::valueChanged);
    int newScale = initialScale + 1;
    archiver.m_ImageSizeScale->setValue(newScale);
    QCOMPARE(sliderSpy.count(), 1);

    // 验证缩放级别更新
    QCOMPARE(archiver.m_thumbnailViewer->GetCurImageSizeScale(), newScale);
    QCOMPARE(archiver.m_ImageSizeScale->value(), newScale);

    // 验证标签文本更新
    QString expectedText = IMAGE_SIZE::HumanReadFriendlySize(newScale);
    QCOMPARE(archiver.m_ImageSizeHint->text(), expectedText);
  }
};

#include "ArchiverTest.moc"
REGISTER_TEST(ArchiverTest, false)
