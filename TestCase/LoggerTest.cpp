#include <QCoreApplication>
#include <QtTest>
#include "GlbDataProtect.h"
#include "TDir.h"
#include "PublicVariable.h"
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "BeginToExposePrivateMember.h"
#include "Logger.h" // ahead of any other file include this file
#include "EndToExposePrivateMember.h"


class LoggerTest : public PlainTestSuite {
  Q_OBJECT
public:
  LoggerTest() : PlainTestSuite{} {}
  TDir mDir;
  const QString LOCAL_LOG{"log_handler_local.log"};
  const QString LOCAL_LOG_ABS_PATH{mDir.itemPath(LOCAL_LOG)};
private slots:
  void test_log_skip_log_file_not_exist() {
    // in debug mode, it should always return stdout;
#ifdef QT_DEBUG
    QVERIFY(Logger::GetFILEStream() == stdout);
    QVERIFY(Logger::mLogFILEStreamUniquePtr == nullptr);
#else // other mode, return a file stream pointer
    QVERIFY(Logger::GetFILEStream() != stdout);
    QVERIFY(Logger::mLogFILEStreamUniquePtr != nullptr);
#endif
    const QString logAbsFilePath = Logger::GetLogFileAbsPath();
    QVERIFY(QFile::exists(logAbsFilePath));

    QVERIFY(logAbsFilePath.endsWith(Logger::CONSTANT_LOG_FILE_NAME, Qt::CaseInsensitive));
  }

  void close_file_stream_only() {
    QCOMPARE(Logger::CloseLogFile(nullptr), false);
    QCOMPARE(Logger::CloseLogFile(stdout), false);
    QCOMPARE(Logger::CloseLogFile(stderr), false);
  }

  void test_set_debug_log_also_flush_instantly() {
    GlbDataProtect<LOG_LVL_E> autoFlushLevelBkp{Logger::m_autoFflushLevel};
    ON_SCOPE_EXIT{
      Logger::out() = Logger::GetFILEStream();
    };

    Logger::SetAutoFlushAllLevel(true);
    Logger::out() = Logger::SwitchLogToALocalFile(LOCAL_LOG_ABS_PATH);
    QVERIFY(Logger::out() != stdout);

    LOG_D("if level under debug not call fflush");
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));
    const QList<FsNodeEntry> entries{mDir.getEntries(true, QDir::Filter::Files)};
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.front().contents.contains("if level under debug not call fflush"));
  }

  void skipAged_if_logFileSize_below_threshold() {
    ON_SCOPE_EXIT{
      Logger::out() = Logger::GetFILEStream();
    };

    const QByteArray contents{"0123456789"};
    QVERIFY(mDir.touch(LOCAL_LOG, contents));
    QVERIFY(contents.size() < 2000);

    QString agedFileName;
    QVERIFY(Logger::AgingLogFiles(LOCAL_LOG_ABS_PATH, 2000, &agedFileName));
    QCOMPARE(agedFileName, "");
    QVERIFY(!mDir.fileExists(agedFileName, false));
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));
  }

  void agedOk_if_logFileSize_above_threshold() {
    ON_SCOPE_EXIT{
      Logger::out() = Logger::SwitchLogToALocalFile();
    };

    const QByteArray contents{"0123456789"};
    QVERIFY(mDir.touch(LOCAL_LOG, contents));
    QVERIFY(contents.size() > 8);

    QString agedFileName;
    QVERIFY(Logger::AgingLogFiles(LOCAL_LOG_ABS_PATH, 8, &agedFileName));
    QVERIFY(agedFileName.endsWith(LOCAL_LOG));
    QVERIFY(agedFileName.size() > LOCAL_LOG.size() + 8 + 6); // at least yyyy-MM-dd_hh_mm_ss_
    QVERIFY(mDir.fileExists(agedFileName, false));
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));
  }
};
#include "LoggerTest.moc"
REGISTER_TEST(LoggerTest, false)
