#include <QDir>
#include <QFileInfo>
#include <QString>
#include "PublicTool.h"

class FileSystemRelatedTest : public QObject {
  Q_OBJECT
 public:
  FileSystemRelatedTest(const char* testSuiteName, const char* testName)
      : TEST_SRC_DIR{QDir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath())
                         .absoluteFilePath(QString("test/%1/DONT_CHANGE").arg(testSuiteName))},
        TEST_DIR{QDir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath())
                     .absoluteFilePath(QString("test/%1/%2").arg(testSuiteName).arg(testName))} {}
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
    auto ret = PublicTool::copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
    assert(ret);  // should copied ok
  }
  void cleanup() {
    if (QDir(TEST_DIR).exists()) {
      QDir(TEST_DIR).removeRecursively();
    }
  }

 protected:
  const QString TEST_SRC_DIR;
  const QString TEST_DIR;
};
