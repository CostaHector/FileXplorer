#include <QtTest>
#include "FileSystemHelper.h"
#include "PlainTestSuite.h"
class FileSystemTestSuite : public PlainTestSuite {
  Q_OBJECT
 public:
  FileSystemTestSuite(const char* testSuiteName, bool autoCleanUp = true);
  ~FileSystemTestSuite();
 protected:
  const QString mTestPath;
  bool mAutoCleanUp;
  const FileSystemHelper m_rootHelper;
 public slots:
  void cleanup();
};
