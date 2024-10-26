#include <QDir>
#include <QFileInfo>
#include <QString>
#include "FileSystemHelper.h"
#include <QtTest>

class FileSystemRelatedTest : public QObject {
  Q_OBJECT
 public:
  FileSystemRelatedTest(const char* testSuiteName, bool autoEnvClear = true)
      : QObject(),
        ROOT_DIR{QDir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath()).absoluteFilePath(QString("test/%1").arg(testSuiteName))},
        m_rootHelper{ROOT_DIR},
        m_autoEnvClear{autoEnvClear} {
    if (!QFileInfo(ROOT_DIR).isDir()) {
      if (!QDir().mkpath(ROOT_DIR)) {
        qWarning("mkpath ROOT_DIR(%s) failed", qPrintable(ROOT_DIR));
        return;
      }
    }
  }
  ~FileSystemRelatedTest() {
    if (!QFileInfo(ROOT_DIR).isDir()) {
      return;
    }
    QFileInfo fi{ROOT_DIR};
    if (!QDir(fi.absolutePath()).rmdir(fi.fileName())) {
      qWarning("remove ROOT_DIR(%s) failed", qPrintable(ROOT_DIR));
    }
  }
  const QString ROOT_DIR;
  const FileSystemHelper m_rootHelper;
  bool m_autoEnvClear;
 public slots:
  void cleanup() {
    if (m_autoEnvClear) {
      QVERIFY(m_rootHelper.EraseFileSystemTree(false));
    }
  }
};
