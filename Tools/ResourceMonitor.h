#ifndef RESOURCEMONITOR_H
#define RESOURCEMONITOR_H
#include <QTimer>
#include <QFile>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <sys/resource.h>
#include <unistd.h>
#endif

class ResourceMonitor : public QObject {
  Q_OBJECT
public:
  explicit ResourceMonitor(const QString& csvFileLocatedIn, QObject *parent = nullptr);
  ~ResourceMonitor();
  ResourceMonitor(const ResourceMonitor& rhs) = delete;
  operator bool() const noexcept {
    return mValid;
  }
  QString GetCsvFileName() const {
    return mCpuMemoryCsvFile;
  }
  static int GetNumbersCore();

public slots:
  void onStart();
  void onStop();
  void onMeasureUsage();

private:
  void WriteCpuMemoryIntoCsvFile(double cpuUsage, double memUsage);
  // 获取进程内存使用 unit: kB
  double getMemoryUsage();
  // 获取进程累计CPU时间 unit: million seconds
  quint64 getProcessCpuTime();

  QString mCpuMemoryCsvFile;
  QTextStream mCpuMemoryCsvStream;
  QFile mCsvFile;

  QMetaObject::Connection mTimeroutConn;
  QTimer m_timer;
  quint64 m_lastCpuTime = 0;

  bool mValid{true};

  int mSampleTimes = 0;
  static constexpr int SAMPLE_PERIOD = 5000;
};

#endif // RESOURCEMONITOR_H
