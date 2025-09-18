#include "FileSystemTestSuite.h"
#include <QDir>
#include <QFileInfo>
#include <QString>

FileSystemTestSuite::FileSystemTestSuite(const char* testSuiteName, bool autoCleanUp)
    : PlainTestSuite{},                                                 //
      mTestPath{QString{""} + TESTCASE_ROOT_PATH "/test/" + testSuiteName},  //
      mAutoCleanUp{autoCleanUp},                                 //
      m_rootHelper{mTestPath}                                    //
{
  if (!QFileInfo{mTestPath}.isDir()) {
    if (!QDir{}.mkpath(mTestPath)) {
      qWarning("mkpath mTestPath(%s) failed", qPrintable(mTestPath));
      return;
    }
  }
}
FileSystemTestSuite::~FileSystemTestSuite() {
  if (!QFileInfo(mTestPath).isDir()) {
    return;
  }
  const QFileInfo fi{mTestPath};
  const QString testPathParentFolder{fi.absolutePath()};
  const QString testFolder{fi.fileName()};
  if (!QDir{testPathParentFolder}.rmdir(testFolder)) {
    qWarning("remove mTestPath(%s) failed", qPrintable(mTestPath));
  }
}

void FileSystemTestSuite::cleanup() {
  if (mAutoCleanUp) {  // auto called after every test
    QVERIFY(m_rootHelper.EraseFileSystemTree(false));
  }
}
