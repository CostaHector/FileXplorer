#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "TDir.h"
#include "CreateFileFolderHelper.h"
#include "JsonHelper.h"
#include "JsonKey.h"

class CreateFileFolderHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mDir;
  const QString tPath{mDir.path()};
private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
  }

  void invalid_input_params() {
    // 测试 NewPlainTextFile 的异常分支
    QString invalidDir = "/non/existent/path";
    QString resultPath;
    QVERIFY(!CreateFileFolderHelper::NewPlainTextFile(invalidDir, &resultPath));
    QVERIFY(resultPath.isEmpty());

    // 测试 NewJsonFile 的异常分支
    QStringList emptyList;
    int result = CreateFileFolderHelper::NewJsonFile(invalidDir, emptyList);
    QCOMPARE(result, 0);

    // 测试 NewFolder 的异常分支
    QVERIFY(!CreateFileFolderHelper::NewFolder(invalidDir));

    // 测试 NewItems 的异常分支
    // 无效范围测试
    QVERIFY(!CreateFileFolderHelper::NewItems(invalidDir, "test_%d.txt", 10, 5)); // start > end

    // 空操作测试
    QVERIFY(!CreateFileFolderHelper::NewItems(invalidDir, "test.txt", 1, 1)); // start == end
  }

  void test_NewPlainTextFile_ok() {
    QString textAbsPath;
    QVERIFY(CreateFileFolderHelper::NewPlainTextFile(tPath, &textAbsPath));
    QVERIFY(textAbsPath.startsWith(tPath + "/New Text Document"));
    QVERIFY(textAbsPath.endsWith(".txt"));
  }
  void test_NewJsonFile_ok() {
    QVERIFY(mDir.ClearAll());

    QStringList basedOnFileNames{"0.mp4", "1.mkv", "3.jpg", "4.json"};
    const QSet<QString> expectSnapShot{
        "0.json",
        "1.json",
    };
    QCOMPARE(CreateFileFolderHelper::NewJsonFile(tPath, basedOnFileNames), 2); // json are only for 2 videos file
    QCOMPARE(mDir.Snapshot(), expectSnapShot);

    QString jsonAbsPath = mDir.itemPath("0.json");
    QVariantHash json0 = JsonHelper::MovieJsonLoader(jsonAbsPath);
    using namespace JsonKey;
    QCOMPARE(json0.value(ENUM_2_STR(Name), "").toString(), "0");
  }
  void test_NewFolder_ok() {
    QVERIFY(mDir.ClearAll());

    QString folderAbsPath;
    QVERIFY(CreateFileFolderHelper::NewFolder(tPath, &folderAbsPath));
    QVERIFY(folderAbsPath.startsWith(tPath + "/New Folder"));
  }
  void test_NewItems_ok() {
    QVERIFY(mDir.ClearAll());

    QVERIFY(!CreateFileFolderHelper::NewItems(tPath, "Page %03d", 1, 1, true));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d.txt", 1, 3, false));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d.txt", 2, 4, false));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d", 1, 3, true));

    const QSet<QString> expectSnapshots{"Page 001", "Page 002", "Page 001.txt", "Page 002.txt", "Page 003.txt"};
    QCOMPARE(mDir.Snapshot(), expectSnapshots);
  }
};

#include "CreateFileFolderHelperTest.moc"
REGISTER_TEST(CreateFileFolderHelperTest, false)
