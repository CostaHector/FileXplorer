#ifndef SYSTERMINAL_H
#define SYSTERMINAL_H

#include <QString>
class SysTerminal {
 public:
  SysTerminal() = default;
  bool operator()(const QString& path);
};

#endif  // SYSTERMINAL_H
