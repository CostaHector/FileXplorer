#include "ResourceMonitor.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QDir>
#include <QDateTime>

constexpr int ResourceMonitor::SAMPLE_PERIOD;

ResourceMonitor::ResourceMonitor(const QString& csvFileLocatedIn, QObject *parent)//
  : QObject{parent} {
  mValid = QDir(csvFileLocatedIn).exists();
  CHECK_FALSE_RETURN_VOID(mValid);

  char timestamp[LOG_TIME_PATTERN_LEN]{0};
  get_timestamp(timestamp, LOG_TIME_PATTERN_LEN);
  mCpuMemoryCsvFile = QString::asprintf("cpu_memory_%s.csv", timestamp);
  mCpuMemoryCsvFile.replace('/', '_');
  mCpuMemoryCsvFile.replace(':', '_');

  mCsvFile.setFileName(QDir(csvFileLocatedIn).absoluteFilePath(mCpuMemoryCsvFile));
  mValid = mCsvFile.open(QIODevice::WriteOnly|QIODevice::Append); // no text here
  CHECK_FALSE_RETURN_VOID(mValid);

  mCpuMemoryCsvStream.setDevice(&mCsvFile);
  mValid = mCpuMemoryCsvStream.status() == QTextStream::Status::Ok;
  CHECK_FALSE_RETURN_VOID(mValid);
  mCpuMemoryCsvStream << "Timestamp,CPU(%),Memory(KB)\n";
  mCpuMemoryCsvStream.flush();
}

ResourceMonitor::~ResourceMonitor() {
  if (m_timer.isActive()) {
    m_timer.stop();
  }
  if (mTimeroutConn) {
    ResourceMonitor::disconnect(mTimeroutConn);
  }
  if (mCpuMemoryCsvStream.device() != nullptr) {
    mCpuMemoryCsvStream.flush();
    mCpuMemoryCsvStream.setDevice(nullptr);
  }
  if (mCpuMemoryCsvStream.device() != nullptr) {
    mCpuMemoryCsvStream.flush();
  }
}

void ResourceMonitor::onStart() {
  m_lastCpuTime = getProcessCpuTime();
#ifndef RUNNING_UNIT_TESTS
  mTimeroutConn = connect(&m_timer, &QTimer::timeout, this, &ResourceMonitor::onMeasureUsage);
  m_timer.start(SAMPLE_PERIOD);
#endif
}

void ResourceMonitor::onStop() {
  if (m_timer.isActive()) {
    m_timer.stop();
  }
  if (mTimeroutConn) {
    ResourceMonitor::disconnect(mTimeroutConn);
  }
}

void ResourceMonitor::WriteCpuMemoryIntoCsvFile(double cpuUsage, double memUsage) {
  char timestamp[LOG_TIME_PATTERN_LEN]{0};
  get_timestamp(timestamp, LOG_TIME_PATTERN_LEN);
  mCpuMemoryCsvStream << timestamp << ","
                      << QString::number(cpuUsage, 'f', 2) << ","
                      << QString::number(memUsage, 'f', 2) << "\n";
#ifdef RUNNING_UNIT_TESTS
  mCpuMemoryCsvStream.flush();
  return;
#endif
  if (++mSampleTimes % 10 == 0) {
    // flush every 10 * SAMPLE_PERIOD
    mCpuMemoryCsvStream.flush();
  }
}

int ResourceMonitor::GetNumbersCore() {
#ifdef Q_OS_WIN
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return sysInfo.dwNumberOfProcessors;
#elif defined(Q_OS_LINUX)
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void ResourceMonitor::onMeasureUsage() {
  // memory usage in KB
  const double memUsage = getMemoryUsage();

  // cpu usage rate
  double cpuUsage = 0.0;
  {
    quint64 currentCpuTime = getProcessCpuTime();
    if (m_lastCpuTime != 0) { // m_lastCpuTime should not be empty
      quint64 cpuDelta = currentCpuTime - m_lastCpuTime;
      static const int numCores = GetNumbersCore();
      static const quint64 maxCpuTime = SAMPLE_PERIOD * numCores;  // max cpu available time in ms
      if (maxCpuTime > 0) {
        cpuUsage = (static_cast<double>(cpuDelta) / maxCpuTime) * 100.0;
      }
    }
    m_lastCpuTime = currentCpuTime;
  }
  LOG_D("%f,%f", cpuUsage, memUsage);
  WriteCpuMemoryIntoCsvFile(cpuUsage, memUsage);
}

double ResourceMonitor::getMemoryUsage() {
#ifdef Q_OS_WIN
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    return pmc.WorkingSetSize / 1024.0; // 总工作集 kB
  }
  // PROCESS_MEMORY_COUNTERS_EX pmc;
  // if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
  //   return pmc.PrivateUsage / 1024.0; // 专用工作集 KB
  // }
#elif defined(Q_OS_LINUX)
  static const QRegExp MEMORY_PATTERN("\\d+");
  QFile file("/proc/self/status");
  if (file.open(QIODevice::ReadOnly)) {
    while (!file.atEnd()) {
      QByteArray line = file.readLine();
      if (line.startsWith("VmRSS:")) {
        return line.split(' ').filter(MEMORY_PATTERN).first().toDouble();
      }
    }
  }
#endif
  return 0.0;
}

quint64 ResourceMonitor::getProcessCpuTime() {
#ifdef Q_OS_WIN
  FILETIME createTime, exitTime, kernelTime, userTime;
  if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime)) {
    ULARGE_INTEGER ulUser, ulKernel;

    // 用户时间
    ulUser.LowPart = userTime.dwLowDateTime;
    ulUser.HighPart = userTime.dwHighDateTime;

    // 内核时间
    ulKernel.LowPart = kernelTime.dwLowDateTime;
    ulKernel.HighPart = kernelTime.dwHighDateTime;

    // 返回总 CPU 时间（毫秒）
    return (ulUser.QuadPart + ulKernel.QuadPart) / 10000;
  }
#elif defined(Q_OS_LINUX)
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage) == 0) {
    // 包含用户时间和系统时间
    quint64 userTime = (usage.ru_utime.tv_sec * 1000) + (usage.ru_utime.tv_usec / 1000);
    quint64 systemTime = (usage.ru_stime.tv_sec * 1000) + (usage.ru_stime.tv_usec / 1000);
    return userTime + systemTime;
  }
#endif
  return 0;
}
