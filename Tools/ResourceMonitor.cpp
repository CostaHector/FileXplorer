#include "ResourceMonitor.h"
#include "Logger.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <sys/resource.h>
#include <unistd.h>
#include <QFile>
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
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_W("Open failed[%s]", qPrintable(file.fileName()));
    return 0.0;
  }
  double memoryKB = 0;
  QByteArray line;
  while (!(line = file.readLine()).isNull()) {
    if (bPrivate) {
      short itemTypesCntMet = 0;
      // 接近Windows的Private Working Set
      if (line.startsWith("VmData:") || line.startsWith("VmStk:") || line.startsWith("VmExe:")) { // VmData:	   81104 kB
        const QStringList& parts = QString::fromUtf8(line).split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 3) {
          memoryKB += parts[1].toDouble();
        }
        if (++itemTypesCntMet == 3) {
          break;
        }
      }
    } else {
      // 获取物理内存使用（驻留集大小Resident Set Size）
      if (line.startsWith("VmRSS:")) { // VmRSS:	   86780 kB
        const QStringList& parts = QString::fromUtf8(line).split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 3) {
          return parts[1].toDouble(); // 直接返回单位 kB
        }
      }
    }
  }
  return memoryKB;
  // double memoryKB = 0;
  // char line[256]{0};
  // FILE* fp = fopen("/proc/self/status", "r");
  // if (fp == nullptr) {
  //   LOG_W("Failed to open /proc/self/status");
  //   return 0.0;
  // }
  // if (bPrivate) { // 读取私有内存：VmData + VmStk + VmExe
  //   int itemTypesCntMet = 1;
  //   const char* ptr = nullptr;
  //   while (fgets(line, sizeof(line), fp)) {
  //     if (strncmp(line, "VmData:", 7) == 0) {
  //       ptr = line + 7;
  //       ++itemTypesCntMet;
  //     } else if (strncmp(line, "VmStk:", 6) == 0) {
  //       ptr = line + 6;
  //       ++itemTypesCntMet;
  //     } else if (strncmp(line, "VmExe:", 6) == 0) {
  //       ptr = line + 6;
  //       ++itemTypesCntMet;
  //     } else {
  //       continue;
  //     }
  //     while (*ptr == ' ' || *ptr == '\t') {
  //       ptr++; // skip empty
  //     }
  //     double value = 0;
  //     if (sscanf(ptr, "%lf", &value) == 1) {
  //       memoryKB += value;
  //     }
  //     if (itemTypesCntMet == 3) {
  //       break;
  //     }
  //   }
  //   fclose(fp);
  //   return memoryKB;
  // } else { // 读取物理内存 VmRSS
  //   while (fgets(line, sizeof(line), fp)) {
  //     if (strncmp(line, "VmRSS:", 6) == 0) {
  //       const char* ptr = line + 6;
  //       while (*ptr == ' ' || *ptr == '\t') {
  //         ptr++; // skip empty blank
  //       }
  //       if (sscanf(ptr, "%lf", &memoryKB) == 1) {
  //         break;
  //       }
  //     }
  //   }
  //   fclose(fp);
  //   LOG_W("VmRSS not found in /proc/self/status");
  //   return 0.0;
  // }
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

}  // namespace ResourceMonitor
