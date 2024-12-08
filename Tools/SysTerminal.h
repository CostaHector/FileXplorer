#ifndef SYSTERMINAL_H
#define SYSTERMINAL_H

#include <QString>
class SysTerminal {
 public:
  SysTerminal() = default;
  bool operator()(const QString& path);
 private:
#ifdef _WIN32
  QString UpdateBatFile(const QString& command) const;
#endif
};

#endif  // SYSTERMINAL_H
