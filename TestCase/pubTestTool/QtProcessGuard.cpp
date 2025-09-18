#include "QtProcessGuard.h"
#include <QProcess>
#include <QCoreApplication>
#include "Logger.h"
#include <QFile>
#include <QTextStream>

constexpr char QtProcessGuard::PID_FILE_PATH[];

// bash /home/ariel/code/FileXplorer/TestCase/pubTestTool/QtProgramGuard.sh 40 'qt' "start" & disown
QtProcessGuard::QtProcessGuard(int timeoutSec, const QString& processPattern, QObject* parent)
    : QObject(parent),            //
      m_timeoutSec(timeoutSec) {  //
  if (timeoutSec <= 0 || processPattern.isEmpty()) {
    LOG_E("Cannot start guard timeoutSec[%d], processPattern[%s]", timeoutSec, qPrintable(processPattern));
    return;
  }
#ifdef _WIN32
  LOG_W("not support in windows");
  return;
#endif
  const QString scriptPath{TESTCASE_ROOT_PATH "/pubTestTool/QtProgramGuard.sh"};
  if (!QFile::exists(scriptPath)) {
    LOG_C("bash file[%s] not found", qPrintable(scriptPath));
    return;
  }
  mStartGuard = QString("bash %1 %2 '%3' \"start\" & disown").arg(scriptPath).arg(m_timeoutSec).arg(processPattern);
  mStopGuard = QString("bash %1 %2 '%3' \"end\" & disown").arg(scriptPath).arg(m_timeoutSec).arg(processPattern);
  int startResult = system(mStartGuard.toUtf8().constData());  // start guard pid
  LOG_D("[Start Guard] system call(%s) return %d", qPrintable(mStartGuard), startResult);
}

QtProcessGuard::~QtProcessGuard() {
  if (mStopGuard.isEmpty()) {
    return;
  }
#ifdef _WIN32
  return;
#endif
  int endResult = system(mStopGuard.toUtf8().constData());  // start guard pid
  LOG_D("[Stop Guard] system call(%s) return %d", qPrintable(mStopGuard), endResult);
}
