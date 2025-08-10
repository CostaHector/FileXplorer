#include <QtTest>
#include "FileSystemHelper.h"
#include "MyTestSuite.h"
class FileSystemTestSuite : public MyTestSuite {
  Q_OBJECT
 public:
  FileSystemTestSuite(const char* testSuiteName, bool autoCleanUp = true, bool bExculsive=false);
  ~FileSystemTestSuite();
 protected:
  const QString mTestPath;
  bool mAutoCleanUp;
  const FileSystemHelper m_rootHelper;
 public slots:
  void cleanup();
};
