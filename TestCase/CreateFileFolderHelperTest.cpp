#include <QCoreApplication>
#include <QtTest>
#include "CreateFileFolderHelper.h"
#include "PlainTestSuite.h"
#include "TDir.h"
#include "JsonHelper.h"
#include "JsonKey.h"

class CreateFileFolderHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  CreateFileFolderHelperTest() : PlainTestSuite{} {
  }

private slots:
  void test_NewPlainTextFile_ok() {
    TDir mDir;
    QString tPath{mDir.path()};
    QString textAbsPath;
    QVERIFY(CreateFileFolderHelper::NewPlainTextFile(tPath, &textAbsPath));
    QVERIFY(textAbsPath.startsWith(tPath + "/New Text Document"));
    QVERIFY(textAbsPath.endsWith(".txt"));
  }
  void test_NewJsonFile_ok() {
    TDir mDir;
    QString tPath{mDir.path()};
    QStringList basedOnFileNames{"0.txt", "1.jpeg", "2.torrent", "3.mp4", "4.pson", "5.json"};
    QStringList expectFileNames {
        "0.json", "1.json", "2.torrent.json", "3.json", "4.json", "5.json"
    };
    QCOMPARE(CreateFileFolderHelper::NewJsonFile(tPath, basedOnFileNames), 6); // 6
    QStringList acutalList = mDir.entryList(QDir::Filter::Files, QDir::SortFlag::Name);
    QCOMPARE(acutalList, expectFileNames);

    QString jsonAbsPath = mDir.itemPath("2.torrent.json");
    QVariantHash json0 = JsonHelper::MovieJsonLoader(jsonAbsPath);
    using namespace JsonKey;
    QCOMPARE(json0.value(ENUM_2_STR(Name), "").toString(), "2.torrent");
  }
  void test_NewFolder_ok() {
    TDir mDir;
    QString tPath{mDir.path()};
    QString folderAbsPath;
    QVERIFY(CreateFileFolderHelper::NewFolder(tPath, &folderAbsPath));
    QVERIFY(folderAbsPath.startsWith(tPath + "/New Folder"));
  }
  void test_NewItems_ok() {
    TDir mDir;
    QString tPath{mDir.path()};
    QVERIFY(!CreateFileFolderHelper::NewItems(tPath, "Page %03d", 1, 1, true));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d.txt", 1, 3, false));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d.txt", 2, 4, false));
    QVERIFY(CreateFileFolderHelper::NewItems(tPath, "Page %03d", 1, 3, true));
    QStringList expectFileNames {
        "Page 001", "Page 002", "Page 001.txt", "Page 002.txt", "Page 003.txt"
    };
    QStringList acutalList = mDir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Files,
                                            QDir::SortFlag::DirsFirst | QDir::SortFlag::Name);
    QCOMPARE(acutalList, expectFileNames);
  }
};

#include "CreateFileFolderHelperTest.moc"
REGISTER_TEST(CreateFileFolderHelperTest, false)
