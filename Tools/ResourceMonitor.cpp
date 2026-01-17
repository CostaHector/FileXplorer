#include "ResourceMonitor.h"
#include "Logger.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace ResourceMonitor {

int GetNumbersCore() {
#ifdef Q_OS_WIN
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return sysInfo.dwNumberOfProcessors;
#elif defined(Q_OS_LINUX)
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
// return value unit: kB
double getMemoryUsage(const bool bPrivate) {
#ifdef Q_OS_WIN
  HANDLE hProcess = GetCurrentProcess();
  if (bPrivate) {
    // Private Bytes 进程分配的私有内存总量，包括提交的虚拟内存。
    PROCESS_MEMORY_COUNTERS_EX pmcex;
    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmcex, sizeof(pmcex))) {
    }
    return pmcex.PrivateUsage / 1024.0;
    // Private Working Set：工作集中私有（非共享）的部分。
  } else {
     // Working Set 进程当前在物理内存中的内存。
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
      return 0.0;
    }
    return pmc.WorkingSetSize / 1024.0;
  }
#elif defined(Q_OS_LINUX)
  QFile file("/proc/self/status");
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_W("Open failed[%s]", qPrintable(file.fileName()));
    return 0.0;
  }
  double memoryKB = 0;
  while (!file.atEnd()) {
    const QByteArray& line = file.readLine();
    if (bPrivate) {
      // 接近Windows的Private Working Set
      if (line.startsWith("VmData:") || line.startsWith("VmStk:") || line.startsWith("VmExe:")) {
        const QStringList& parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
          memoryKB += parts[1].toDouble();
        }
      }
    } else {
      // 获取物理内存使用（驻留集大小Resident Set Size）
      if (line.startsWith("VmRSS:")) {
        const QList<QByteArray>& parts = line.split(' ', Qt::SkipEmptyParts);
        for (const QByteArray &part : parts) {
          if (part[0] >= '0' && part[0] <= '9') {
            return part.toDouble(); // 直接返回单位 kB
          }
        }
      }
    }
  }
  return memoryKB;
#endif
  return 0.0;
}
// cpu used
quint64 getProcessCpuTime() {
#ifdef Q_OS_WIN
  HANDLE hProcess = GetCurrentProcess();
  FILETIME createTime, exitTime, kernelTime, userTime;
  if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
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

}
