#ifndef SYSTERMINAL_H
#define SYSTERMINAL_H

#include <QString>
class SysTerminal {
 public:
  SysTerminal();

  bool WriteIntoBatchFile(const QString& command);

  bool operator()(const QString& path);

  QString m_BAT_FILE_PATH;
};

#endif  // SYSTERMINAL_H
