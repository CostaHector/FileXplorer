#ifndef ITEMSPACKER_H
#define ITEMSPACKER_H

#include "FileOperatorPub.h"
#include <QString>

class ItemsPacker {
 public:
  int operator()(const QString& path, const QMap<QString, QStringList>& pilesMap);
  int operator()(const QString& path);
  bool StartToRearrange();
  int CommandsCnt() const { return m_cmds.size(); }

 private:
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};

#endif  // ITEMSPACKER_H
