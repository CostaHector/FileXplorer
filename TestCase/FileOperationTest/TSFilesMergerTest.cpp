#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "PathRelatedTool.h"
#include "TSFilesMerger.h"
#include <QFile>
#include <QDir>

class TSFilesMergerTest : public MyTestSuite {
  Q_OBJECT

public:
  TSFilesMergerTest() : MyTestSuite{false} {}
  const QString rootpath = TestCaseRootPath() + "/test/TestEnv_TSFilesMerger";
  const QString mOutputLargeTSFileName{"TestEnv_TSFilesMerger.ts"};
  const QString mOutputLargeTSAbsFileName{rootpath + "/" + mOutputLargeTSFileName};
private slots:
  void cleanup() {
    if (QFile::exists(mOutputLargeTSAbsFileName)) {
      QVERIFY(QFile::remove(mOutputLargeTSAbsFileName));
    }
  }

  void test_checkTsFilesConsistent_ok() {
    QDir dir{rootpath};
    const QStringList tsFilesAbsPath {dir.absoluteFilePath("File need to merge seg-1-v1-a1.ts"),//
                                     dir.absoluteFilePath("Files no need to merge sample-ts-files-sample_640x360.ts")};
    QVERIFY(!TSFilesMerger::checkTsFilesConsistent(tsFilesAbsPath));
    // uhmmm {"File need to merge seg-1-v1-a1.ts", "File need to merge seg-2-v1-a1.txt"} also return false
  }

  void test_mergeTsFiles_skip() {
    QStringList tsFiles {"File need to merge seg-1-v1-a1.ts"};
    QString largeTsFileAbsPath;
    bool mergeResult {true};

    // only one file
    mergeResult = true;
    std::tie(mergeResult, largeTsFileAbsPath) = TSFilesMerger::mergeTsFiles(rootpath, tsFiles);
    QVERIFY(!mergeResult);

    // path not exist
    mergeResult = true;
    tsFiles = QStringList {"File need to merge seg-1-v1-a1.ts",//
                          "File need to merge seg-2-v1-a1.ts"};
    std::tie(mergeResult, largeTsFileAbsPath) = TSFilesMerger::mergeTsFiles("invalid rootpath", tsFiles);
    QVERIFY(!mergeResult);

    // not ts file
    mergeResult = true;
    tsFiles = QStringList {"File need to merge seg-1-v1-a1.txt",//
                          "File need to merge seg-2-v1-a1.txt"};
    std::tie(mergeResult, largeTsFileAbsPath) = TSFilesMerger::mergeTsFiles(rootpath, tsFiles);
    QVERIFY(!mergeResult);
  }

  void test_mergeTsFiles() {
    const QStringList tsFiles {
                              "File need to merge seg-1-v1-a1.txt",
                              "File need to merge seg-2-v1-a1.txt",
                              "File need to merge seg-1-v1-a1.ts",//
                              "File need to merge seg-2-v1-a1.ts"};
    QString largeTsFileAbsPath;
    bool mergeResult {false};
    std::tie(mergeResult, largeTsFileAbsPath) = TSFilesMerger::mergeTsFiles(rootpath, tsFiles);
    QVERIFY(mergeResult);
    QCOMPARE(largeTsFileAbsPath, mOutputLargeTSAbsFileName);

    // large ts files already exist
    mergeResult = true;
    largeTsFileAbsPath = "";
    std::tie(mergeResult, largeTsFileAbsPath) = TSFilesMerger::mergeTsFiles(rootpath, tsFiles);
    QVERIFY(!mergeResult);
    QCOMPARE(largeTsFileAbsPath, "");
  }
};

#include "TSFilesMergerTest.moc"
TSFilesMergerTest g_TSFilesMergerTest;
