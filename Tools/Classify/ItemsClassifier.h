#ifndef ITEMSCLASSIFIER_H
#define ITEMSCLASSIFIER_H

#include "FileOperation/FileOperatorPub.h"
#include <QString>

class ItemsClassifier {
 public:
  int operator()(const QString& path, const QMap<QString, QStringList>& pilesMap);
  int operator()(const QString& path);
  bool StartToRearrange();
  int CommandsCnt() const { return m_cmds.size(); }

 private:
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};

#endif  // ITEMSCLASSIFIER_H
