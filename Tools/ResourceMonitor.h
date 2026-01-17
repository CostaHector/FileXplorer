#ifndef RESOURCEMONITOR_H
#define RESOURCEMONITOR_H
#include <QtGlobal>

namespace ResourceMonitor {
  int GetNumbersCore();
  // 获取进程内存使用 unit: kB
  double getMemoryUsage(const bool bPrivate=false);
  // 获取进程累计CPU时间 unit: million seconds
  quint64 getProcessCpuTime();
}

#endif // RESOURCEMONITOR_H
