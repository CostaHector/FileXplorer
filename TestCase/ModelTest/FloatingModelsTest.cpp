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
    FloatingModels model{"FloatingListView"};
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.IsEmpty());
    QCOMPARE(model.rootPath(), "");

    QModelIndex invalidIndex;
    QVERIFY(model.data(invalidIndex).isNull());
    QVERIFY(model.filePath(invalidIndex).isEmpty());

    QCOMPARE(model.data(model.index(999), Qt::DisplayRole).isValid(), false);  // out of range

    QVERIFY(model.isOuterBound(-1));
    QVERIFY(model.isOuterBound(0));
    QVERIFY(model.isOuterBound(1));

    QCOMPARE(model.rel2fileNames({}), (QStringList{}));
    QCOMPARE(model.AfterRowsRemoved({}), 0);
  }

  void data_retrieve_ok() {
    QStringList testData{"/path/to/file1.mp4", "/path/to/file2.mkv", "/path/to/file3.avi"};
    VidsModel model{"FloatingListView_VidsModel"};
    QCOMPARE(model.UpdateData(testData), 3);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.rootPath(), "/path/to");

    QCOMPARE(model.data(model.index(0), Qt::DisplayRole).toString(), "file1");
    QCOMPARE(model.data(model.index(1), Qt::DisplayRole).toString(), "file2");
    QCOMPARE(model.data(model.index(2), Qt::DisplayRole).toString(), "file3");
    QCOMPARE(model.data(model.index(0), Qt::DecorationRole).isValid(), true);  // from QIconProvider

    QCOMPARE(model.filePath(model.index(0)), "/path/to/file1.mp4");
    QCOMPARE(model.filePath(model.index(1)), "/path/to/file2.mkv");
    QCOMPARE(model.filePath(model.index(2)), "/path/to/file3.avi");
  }

  void setDirPath_loadAll_ok() {
    QVERIFY(tDir.ClearAll());
    QList<FsNodeEntry> files{
        {"image1.jpg", false, ""},   //
        {"image2.png", false, ""},   //
        {"image3.webp", false, ""},  //
        {"videos.mp4", false, ""},   //
    };
    QCOMPARE(tDir.createEntries(files), 4);

    // loadAllIn1Time
    ImgsModel model{"FloatingListView_ImgsModel"};
    QStringList filters{"*.jpg", "*.png", "*.webp"};
    QCOMPARE(model.setDirPath(tDir.path(), filters, true), 3);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.rootPath(), tDir.path());

    QCOMPARE(model.filePath(model.index(0)), tDir.itemPath("image1.jpg"));
    QCOMPARE(model.filePath(model.index(1)), tDir.itemPath("image2.png"));
    QCOMPARE(model.filePath(model.index(2)), tDir.itemPath("image3.webp"));
  }

  void setDirPath_incremental_load_ok() {
    QVERIFY(tDir.ClearAll());
    QList<FsNodeEntry> nodes{
        {"a.jpg", false, ""},   //
        {"b.png", false, ""},   //
        {"c.webp", false, ""},  //
        {"d.jpg", false, ""},   //
        {"e.png", false, ""},   //
        {"f.webp", false, ""},  //
        {"g.jpg", false, ""},   //
        {"h.png", false, ""},   //
        {"i.webp", false, ""},  //
        {"j.jpg", false, ""},   //
    };
    QCOMPARE(tDir.createEntries(nodes), 10);

    QSet<QString> expectFilesNames;
    for (const FsNodeEntry& node : nodes) {
      expectFilesNames.insert(tDir.itemPath(node.relativePathToNode));
    }

    // 增量加载
    QStringList filters{{"*.jpg", "*.png", "*.webp"}};
    FloatingModels model{"FloatingListView"};
    QCOMPARE(model.setDirPath(tDir.path(), filters, false), 10);

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

    // 验证所有文件路径都存在, 排序先按照长度, 再按照ascii
    QCOMPARE(model.data(model.index(0)).toString(), tDir.itemPath("a.jpg"));
    QCOMPARE(model.data(model.index(1)).toString(), tDir.itemPath("b.png"));
    QCOMPARE(model.data(model.index(2)).toString(), tDir.itemPath("d.jpg"));
    QCOMPARE(model.data(model.index(3)).toString(), tDir.itemPath("e.png"));
    QCOMPARE(model.data(model.index(4)).toString(), tDir.itemPath("g.jpg"));
    QCOMPARE(model.data(model.index(5)).toString(), tDir.itemPath("h.png"));
    QCOMPARE(model.data(model.index(6)).toString(), tDir.itemPath("j.jpg"));
    QCOMPARE(model.data(model.index(7)).toString(), tDir.itemPath("c.webp"));
    QCOMPARE(model.data(model.index(8)).toString(), tDir.itemPath("f.webp"));
    QCOMPARE(model.data(model.index(9)).toString(), tDir.itemPath("i.webp"));
  }

  void ImgsModel_resize_images_ok() {
    // precondition
    QVERIFY(tDir.ClearAll());
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

    QVERIFY(tDir.touch("Chris Evans.jpg", svgContent.toUtf8()));
    const QString validImage{tDir.itemPath("Chris Evans.jpg")};
    const QString notExistImage{"inexist_images.webp"};
    {
      // read from file ok
      QCOMPARE(CastBrowserHelper::GetImageSize(validImage), sourceSvgSize);
    }

    ImgsModel imgModel{"ImgsListView_ImgsModel"};
    const QStringList imgDatas{
        validImage,    //
        notExistImage  //
    };                 //
    QCOMPARE(imgModel.UpdateData(imgDatas), 2);

    const QModelIndex validImageIndex{imgModel.index(0)};
    {
      // 1.0 access this two file by data(). cache get inserted
      QVariant validImageVariant = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant.isValid());
      QVERIFY(validImageVariant.canConvert<QPixmap>());
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
      // QVERIFY(!imgModel.mPixCache.find(validImage, nullptr));
      // 3. check pixmap size
      QVariant validImageVariant1000by10 = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant1000by10.isValid());
      QVERIFY(validImageVariant1000by10.canConvert<QPixmap>());
      QPixmap img1000by360 = validImageVariant1000by10.value<QPixmap>();
      QSize sz = img1000by360.size();
      QCOMPARE(sz, QSize(540 * 2, 360 * 2));
      // QVERIFY(imgModel.mPixCache.find(validImage, nullptr)); todo
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
      // QVERIFY(!imgModel.mPixCache.find(validImage, nullptr));
      // 3. check pixmap size
      QVariant validImageVariant10by1000 = imgModel.data(validImageIndex, Qt::DecorationRole);
      QVERIFY(validImageVariant10by1000.isValid());
      QVERIFY(validImageVariant10by1000.canConvert<QPixmap>());
      QPixmap img540by1000 = validImageVariant10by1000.value<QPixmap>();
      QSize sz = img540by1000.size();
      QCOMPARE(sz, QSize(540 / 2, 360 / 2));
      // QVERIFY(imgModel.mPixCache.find(validImage, nullptr));
    }

    {
      // 5. size not change, pixmap cache not update
      imgModel.onIconSizeChange(sz540by1000);
      // QVERIFY(imgModel.mPixCache.find(validImage, nullptr));
    }
  }

  void vidsModel_ok() {
    VidsModel vmodel{"VidsListView"};
    const QStringList vidDatas{
        "/home/to/Chris Evans.mp4",           //
        "C:/home/to/Michael Fassbender.mp4",  //
        "C:/home/Ben Afleck.mp4",             //
    };                                        //
    QCOMPARE(vmodel.UpdateData(vidDatas), 3);
    QVERIFY(vmodel.data(QModelIndex{}).isNull());                        // out of range
    QVERIFY(vmodel.data(vmodel.index(0, Qt::ForegroundRole)).isNull());  // not specified

    QCOMPARE(vmodel.data(vmodel.index(0, Qt::DisplayRole)).toString(), "Chris Evans");
    QCOMPARE(vmodel.data(vmodel.index(1, Qt::DisplayRole)).toString(), "Michael Fassbender");
    QCOMPARE(vmodel.data(vmodel.index(2, Qt::DisplayRole)).toString(), "Ben Afleck");

    const QVariant iconFromProvider = vmodel.data(vmodel.index(2), Qt::DecorationRole);
    QVERIFY(!iconFromProvider.isNull());
  }

  void othersModel_ok() {
    OthersModel oModel{"VidsListView"};
    const QStringList vidDatas{
        "/home/to/Chris Evans.txt",          //
        "C:/home/to/Michael Fassbender.md",  //
        "C:/home/Ben Afleck.json",           //
    };                                       //
    QCOMPARE(oModel.UpdateData(vidDatas), 3);
    QVERIFY(oModel.data(QModelIndex{}).isNull());                        // out of range
    QVERIFY(oModel.data(oModel.index(0, Qt::ForegroundRole)).isNull());  // not specified

    QCOMPARE(oModel.data(oModel.index(0, Qt::DisplayRole)).toString(), "Chris Evans.txt");
    QCOMPARE(oModel.data(oModel.index(1, Qt::DisplayRole)).toString(), "Michael Fassbender.md");
    QCOMPARE(oModel.data(oModel.index(2, Qt::DisplayRole)).toString(), "Ben Afleck.json");

    const QVariant iconFromProvider = oModel.data(oModel.index(2), Qt::DecorationRole);
    QVERIFY(!iconFromProvider.isNull());
  }

  void rel2fileNames_AfterRowsRemoved_ok() {
    ImgsModel model{"FloatingListView_ImgsModel"};
    QCOMPARE(model.UpdateData(QStringList{"rel2fileNames/a.jpg", "rel2fileNames/b.png", "rel2fileNames/c.gif"}), 3);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.rootPath(), "rel2fileNames");

    QCOMPARE(model.rel2fileNames({}), (QStringList{}));
    QCOMPARE(model.rel2fileNames({model.index(0), model.index(2)}), (QStringList{"a.jpg", "c.gif"}));
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.AfterRowsRemoved({}), 0);
    QCOMPARE(model.AfterRowsRemoved({model.index(0), model.index(2)}), 2);  // remove first and last one
    QCOMPARE(model.mDataLst.size(), 1);
    QCOMPARE(model.rowCount(), 1);  // rowCount() return by m_curLoadedCount

    QCOMPARE(model.filePath(model.index(0)), "rel2fileNames/b.png");
  }
};

#include "FloatingModelsTest.moc"
REGISTER_TEST(FloatingModelsTest, false)
