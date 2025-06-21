#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/GlbDataProtect.h"
#include "pub/MyTestSuite.h"
#include "pub/TDir.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/LogHandler.h"
#include "pub/EndToExposePrivateMember.h"

class LogHandlerTest : public MyTestSuite {
  Q_OBJECT
 public:
  LogHandlerTest() : MyTestSuite{false} {}
  TDir mDir;
  const QString LOCAL_LOG{"log_handler_local.log"};
  const QString LOCAL_LOG_ABS_PATH{mDir.itemPath(LOCAL_LOG)};
 private slots:

  void test_log_skip_log_file_not_exist() {
    LogHandler logHandler{"invalidPath/local.log", QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), false);  // open failed
  }

  void test_log_warning_flush_instantly() {  // no need manual flush
    GlbDataProtect<bool> bkp{LogHandler::mAutoFlushLogBuffer};
    LogHandler::mAutoFlushLogBuffer = false;

    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);

    qWarning("Warning Chris Pine");
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));

    const QList<FsNodeEntry> entries{mDir.getEntries(true, QDir::Filter::Files)};
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.front().contents.contains("Warning Chris Pine"));
  }

  void test_log_debug_output_log_level_debug_no_need_manual_flush() {  // no need manual flush
    GlbDataProtect<bool> bkp{LogHandler::mAutoFlushLogBuffer};
    LogHandler::mAutoFlushLogBuffer = false;
    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);
    GlbDataProtect<QtMsgType> outputLogLevelBkp{LogHandler::OUTPUT_LOG_LEVEL};
    LogHandler::SetLogLevelDebug();

    qDebug("Debug Chris Pine");  // only call flush textstream, not call flush file
                                 // LogHandler::mLogFile.flush();
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));

    const QList<FsNodeEntry> entries{mDir.getEntries(true, QDir::Filter::Files)};
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.front().contents.contains("Debug Chris Pine"));
  }

  void test_log_debug_need_manual_flush() {  // need manual flush
    GlbDataProtect<bool> bkp{LogHandler::mAutoFlushLogBuffer};
    LogHandler::mAutoFlushLogBuffer = false;

    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);

    qDebug("Debug Chris Pine");
    logHandler.ManualFlush();
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));

    const QList<FsNodeEntry> entries{mDir.getEntries(true, QDir::Filter::Files)};
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.front().contents.contains("Debug Chris Pine"));
  }

  void test_log_debug_auto_flush_log_buffer_switch_on() {  // no need manual flush
    GlbDataProtect<bool> bkp{LogHandler::mAutoFlushLogBuffer};
    LogHandler::mAutoFlushLogBuffer = true;

    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);

    qDebug("Debug Chris Pine");  // only call flush textstream, not call flush file
                                 // LogHandler::mLogFile.flush();
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));

    const QList<FsNodeEntry> entries{mDir.getEntries(true, QDir::Filter::Files)};
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.front().contents.contains("Debug Chris Pine"));
  }

  void test_aging_skip_file_size_below_threshold() {
    const QByteArray contents{"0123456789"};
    QVERIFY(mDir.touch(LOCAL_LOG, contents));
    static constexpr int AGING_FILE_THRESHOLDE_SIZE = 2000;  // bytes
    QVERIFY(contents.size() < AGING_FILE_THRESHOLDE_SIZE);

    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);  // open ok

    QString agedFileName;
    QVERIFY(LogHandler::AgingLogFiles(AGING_FILE_THRESHOLDE_SIZE, &agedFileName));
    QVERIFY(agedFileName.isEmpty());
    QVERIFY(!mDir.fileExists(agedFileName, false));
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));
  }

  void test_aging_ok_file_size_greater_than_threshold() {
    const QByteArray contents{"0123456789"};
    QVERIFY(mDir.touch(LOCAL_LOG, contents));
    static constexpr int AGING_FILE_THRESHOLDE_SIZE = 8;  // bytes
    QVERIFY(contents.size() > AGING_FILE_THRESHOLDE_SIZE);

    LogHandler logHandler{LOCAL_LOG_ABS_PATH, QtMsgType::QtWarningMsg};
    QCOMPARE(logHandler.IsLogModuleOk(), true);  // open ok

    QString agedFileName;
    QVERIFY(LogHandler::AgingLogFiles(AGING_FILE_THRESHOLDE_SIZE, &agedFileName));
    QVERIFY(!agedFileName.isEmpty());
    QVERIFY(mDir.fileExists(agedFileName, false));
    QVERIFY(mDir.fileExists(LOCAL_LOG, false));
  }
};
#include "LogHandlerTest.moc"
LogHandlerTest g_LogHandlerTest;
