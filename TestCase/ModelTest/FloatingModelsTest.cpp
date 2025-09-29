#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include "BeginToExposePrivateMember.h"
#include "FloatingModels.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "ImageTestPrecoditionTools.h"
#include "CastBrowserHelper.h"

class FloatingModelsTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {  //
    QVERIFY(tDir.IsValid());
  }

  void initalize_ok() {
    FloatingModels model;
    // 验证初始状态
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.IsEmpty());

    // 调用成员函数不应崩溃
    QModelIndex invalidIndex;
    QVERIFY(model.data(invalidIndex).isNull());
    QVERIFY(model.filePath(invalidIndex).isEmpty());

    // 测试边界情况
    QVERIFY(model.isOuterBound(-1));
    QVERIFY(model.isOuterBound(0));
    QVERIFY(model.isOuterBound(1));
  }

  void data_retrieve_ok() {
    FloatingModels model;

    // 准备测试数据
    QStringList testData = {"/path/to/file1.jpg", "/path/to/file2.png", "/path/to/file3.webp"};

    // 更新模型数据
    model.UpdateData(testData);
    QCOMPARE(model.rowCount(), 3);

    // 验证数据检索
    for (int i = 0; i < 3; i++) {
      QModelIndex idx = model.index(i, 0);
      QVERIFY(idx.isValid());
      QCOMPARE(model.data(idx, Qt::DisplayRole).toString(), testData[i]);
      QCOMPARE(model.filePath(idx), testData[i]);
    }

    QVERIFY(model.data(model.index(0), Qt::DecorationRole).isNull());

    // 验证无效索引
    QModelIndex invalidIdx = model.index(3, 0);
    QVERIFY(!invalidIdx.isValid());
    QVERIFY(model.data(invalidIdx).isNull());
    QVERIFY(model.filePath(invalidIdx).isEmpty());
  }

  void setDirPath_loadAll_ok() {
    QVERIFY(tDir.IsValid());
    QVERIFY(tDir.ClearAll());

    // 创建测试文件
    QList<FsNodeEntry> files = {{"image1.jpg", false, ""}, {"image2.png", false, ""}, {"image3.webp", false, ""}};
    QCOMPARE(tDir.createEntries(files), 3);

    FloatingModels model;

    // 设置目录路径（一次性加载）
    QStringList filters = {"*.jpg", "*.png", "*.webp"};
    int result = model.setDirPath(tDir.path(), filters, true);
    QCOMPARE(result, 3);
    QCOMPARE(model.rowCount(), 3);

    // 验证文件路径排序
    QStringList expectedPaths = {tDir.itemPath("image1.jpg"), tDir.itemPath("image2.png"), tDir.itemPath("image3.webp")};
    for (int i = 0; i < 3; i++) {
      QCOMPARE(model.data(model.index(i, 0)).toString(), expectedPaths[i]);
    }
  }

  void setDirPath_incremental_load_ok() {
    QVERIFY(tDir.IsValid());
    QVERIFY(tDir.ClearAll());

    // 创建测试文件
    QList<FsNodeEntry> nodes = {{"a.jpg", false, ""},  {"b.png", false, ""}, {"c.webp", false, ""}, {"d.jpg", false, ""},  {"e.png", false, ""},
                                {"f.webp", false, ""}, {"g.jpg", false, ""}, {"h.png", false, ""},  {"i.webp", false, ""}, {"j.jpg", false, ""}};
    QCOMPARE(tDir.createEntries(nodes), 10);

    QSet<QString> expectFilesNames;
    for (const FsNodeEntry& node : nodes) {
      expectFilesNames.insert(tDir.itemPath(node.relativePathToNode));
    }

    FloatingModels model;

    // 设置目录路径（增量加载）
    QStringList filters = {"*.jpg", "*.png", "*.webp"};
    int result = model.setDirPath(tDir.path(), filters, false);
    QCOMPARE(result, 10);

    // 初始状态（未加载任何项）
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.canFetchMore(QModelIndex()));

    // 第一次获取更多数据
    model.fetchMore(QModelIndex());
    QCOMPARE(model.rowCount(), FloatingModels::BATCH_LOAD_COUNT);  // equals to 8

    // 第二次获取更多数据
    model.fetchMore(QModelIndex());
    QCOMPARE(model.rowCount(), 10);

    QVERIFY(!model.canFetchMore(QModelIndex()));
    // // 第三次获取更多数据
    // model.fetchMore(QModelIndex());
    // QCOMPARE(model.rowCount(), 10);

    // 验证所有文件路径都存在
    QSet<QString> actualFilesNames;
    for (int i = 0; i < 10; i++) {
      actualFilesNames.insert(model.data(model.index(i, 0)).toString());
    }
    QCOMPARE(actualFilesNames, expectFilesNames);
  }

  void ImgsModel_resize_images_ok() {
    // precondition
    using namespace ImageTestPrecoditionTools;
    const QString svgContentTemplate = GetSvgContentTemplate();
    const QString svgContent = svgContentTemplate.arg(SVG_FILL_COLORS[0]).arg("C");
    // QPixmap source pixel should be 540:360
    const QSize sourceSvgSize{540, 360};
    {
      QPixmap svgPixmap;
      QVERIFY(svgPixmap.loadFromData(svgContent.toUtf8().constData(), ".svg"));
      QCOMPARE(svgPixmap.size(), sourceSvgSize);
    }

    QVERIFY(tDir.IsValid());
    QVERIFY(tDir.ClearAll());
    QVERIFY(tDir.touch("Chris Evans.jpg", svgContent.toUtf8()));
    const QString validImage{tDir.itemPath("Chris Evans.jpg")};
    const QString notExistImage{"inexist_images.webp"};
    {
      // read from file ok
      QCOMPARE(CastBrowserHelper::GetImageSize(validImage), sourceSvgSize);
    }

    ImgsModel imgModel;
    const QStringList imgDatas{
        validImage,    //
        notExistImage  //
    };                 //
    QCOMPARE(imgModel.UpdateData(imgDatas), 2);

    const QModelIndex validImageIndex{imgModel.index(0)};
    const QModelIndex invalidImageIndex{imgModel.index(1)};
    QVERIFY(!imgModel.mPixCache.find(validImage, nullptr));
    QVERIFY(!imgModel.mPixCache.find(notExistImage, nullptr));

    {
      // 1.0 access this two file by data(). cache get inserted
      QVariant validImageVariant = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant.isValid());
      QVERIFY(validImageVariant.canConvert<QPixmap>());
      QVariant inexistImageVariant = imgModel.data(invalidImageIndex, Qt::DecorationRole);
      QVERIFY(!inexistImageVariant.isValid());
      QVERIFY(imgModel.mPixCache.find(validImage, nullptr));  // valid image should in cache now
      QVERIFY(!imgModel.mPixCache.find(notExistImage, nullptr));
    }

    /*
     * 540-by-360  = w/h=1.5
     * 1000-by-360 = w/h=2.78
     * should scale by height
     */
    const QSize sz1000by360{1000 * 2, 360 * 2};
    {
      // 2. change icon size cache will get cleared
      imgModel.onIconSizeChange(sz1000by360);
      QVERIFY(!imgModel.mPixCache.find(validImage, nullptr));
      // 3. check pixmap size
      QVariant validImageVariant1000by10 = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant1000by10.isValid());
      QVERIFY(validImageVariant1000by10.canConvert<QPixmap>());
      QPixmap img1000by360 = validImageVariant1000by10.value<QPixmap>();
      QSize sz = img1000by360.size();
      QCOMPARE(sz, QSize(540 * 2, 360 * 2));
      QVERIFY(imgModel.mPixCache.find(validImage, nullptr));
    }
    /*
     * 540-by-360  = w/h=1.5
     * 540-by-1000 = w/h=0.54
     * should scale by width
     */
    const QSize sz540by1000{540 / 2, 1000 / 2};
    {
      // 4. set icon size to 10-by-1000 pixel
      imgModel.onIconSizeChange(sz540by1000);
      QVERIFY(!imgModel.mPixCache.find(validImage, nullptr));
      // 3. check pixmap size
      QVariant validImageVariant10by1000 = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant10by1000.isValid());
      QVERIFY(validImageVariant10by1000.canConvert<QPixmap>());
      QPixmap img540by1000 = validImageVariant10by1000.value<QPixmap>();
      QSize sz = img540by1000.size();
      QCOMPARE(sz, QSize(540 / 2, 360 / 2));
      QVERIFY(imgModel.mPixCache.find(validImage, nullptr));
    }

    {
      // 5. size not change, pixmap cache not update
      imgModel.onIconSizeChange(sz540by1000);
      QVERIFY(imgModel.mPixCache.find(validImage, nullptr));
    }
  }

  void vidsModel_ok() {
    VidsModel vmodel;
    const QStringList vidDatas{
        "/home/to/Chris Evans.mp4",           //
        "C:/home/to/Michael Fassbender.mp4",  //
        "C:/home/Ben Afleck.mp4",             //
    };                                        //
    QCOMPARE(vmodel.UpdateData(vidDatas), 3);
    QCOMPARE(vmodel.data(vmodel.index(0, Qt::DisplayRole)).toString(), "Chris Evans");
    QCOMPARE(vmodel.data(vmodel.index(1, Qt::DisplayRole)).toString(), "Michael Fassbender");
    QCOMPARE(vmodel.data(vmodel.index(2, Qt::DisplayRole)).toString(), "Ben Afleck");

    const QVariant iconFromProvider = vmodel.data(vmodel.index(2), Qt::DecorationRole);
    QVERIFY(!iconFromProvider.isNull());

    QVERIFY(vmodel.data(QModelIndex{}).isNull());                        // out of range
    QVERIFY(vmodel.data(vmodel.index(0, Qt::ForegroundRole)).isNull());  // not set
  }
};

#include "FloatingModelsTest.moc"
REGISTER_TEST(FloatingModelsTest, false)
