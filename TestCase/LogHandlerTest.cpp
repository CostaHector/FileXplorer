#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Public/PublicTool.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/LogHandler.h"
#include "pub/EndToExposePrivateMember.h"

const QString LOCAL_LOG{"log_handler_local.log"};

class LogHandlerTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_Aging_skip_file_size_below() {
    const QString content{"0123456789"};
    QVERIFY2(TextWriter(LOCAL_LOG, content), qPrintable(LOCAL_LOG));
    QVERIFY2(QFile::exists(LOCAL_LOG), "file should exist");

    QString agedLogPath;
    {
      LogHandler logHandler{nullptr, LOCAL_LOG};
      QVERIFY2(LogHandler::AgingLogFiles(2000, &agedLogPath), "file size too small not above threshold");
      QVERIFY(agedLogPath.isEmpty());
      QVERIFY2(QFile::exists(LOCAL_LOG), "log file should still exist");
    }

    QVERIFY2(QFile::remove(LOCAL_LOG), "log handler file should closed and removed succeed");
  }

  void test_Aging_ok() {
    const QString content{"0123456789"};
    QVERIFY2(TextWriter(LOCAL_LOG, content), qPrintable(LOCAL_LOG));
    QVERIFY2(QFile::exists(LOCAL_LOG), "file should exist");

    QString agedLogPath;
    {
      LogHandler logHandler{nullptr, LOCAL_LOG};
      QVERIFY2(LogHandler::AgingLogFiles(8, &agedLogPath), "file size above threshold 8 bytes, should aged");
      QVERIFY(!agedLogPath.isEmpty());  // LOCAL_LOG_timestamp.log, YYYY-MM-DD_hh_mm_ss
      QVERIFY(agedLogPath.contains(LOCAL_LOG));
      QVERIFY(agedLogPath.endsWith(".log"));
      QVERIFY2(QFile::exists(LOCAL_LOG), "log file should still exist");
      QVERIFY2(QFile::exists(agedLogPath), "aged file should exist");
      QVERIFY2(QFile::remove(agedLogPath), "aged file file should closed and removed succeed");
    }

    QVERIFY2(QFile::remove(LOCAL_LOG), "log handler file should closed and removed succeed");
  }
};
#include "LogHandlerTest.moc"
LogHandlerTest g_LogHandlerTest;
