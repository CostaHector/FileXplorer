#ifndef CATEGORIZER_H
#define CATEGORIZER_H

#include "FileOperation/FileOperatorPub.h"
#include <QString>

class Categorizer {
 public:
  static QMap<QString, QStringList> ClassifyItemIntoPiles(const QString& path);
  static QMap<QString, QStringList> ClassifyItemIntoPiles(const QStringList& files);
  int operator()(const QString& path, const QMap<QString, QStringList>& pilesMap);
  int operator()(const QString& path);
  bool StartToRearrange();
  int CommandsCnt() const { return m_cmds.size(); }

 private:
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};

#endif  // CATEGORIZER_H
