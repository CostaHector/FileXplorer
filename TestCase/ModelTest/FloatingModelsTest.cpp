#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include "BeginToExposePrivateMember.h"
#include "FloatingModels.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class FloatingModelsTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
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
    TDir tDir;
    QVERIFY(tDir.IsValid());

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
    TDir tDir;
    QVERIFY(tDir.IsValid());

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
    QCOMPARE(model.rowCount(), FloatingModels::BATCH_LOAD_COUNT); // equals to 8

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
};

#include "FloatingModelsTest.moc"
REGISTER_TEST(FloatingModelsTest, false)
