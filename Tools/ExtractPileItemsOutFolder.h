#ifndef EXTRACTPILEITEMSOUTFOLDER_H
#define EXTRACTPILEITEMSOUTFOLDER_H

#include "FileOperation/FileOperatorPub.h"
#include <QString>
#include <QMap>

class ExtractPileItemsOutFolder {
 public:
  static QMap<QString, QStringList> UnpackItemFromPiles(const QString& path);
  static QMap<QString, QStringList> UnpackItemFromPiles(const QString& path, const QStringList& folders);

  static bool CanExtractOut(const QStringList& items);
  int operator()(const QString& path);
  int operator()(const QString& path, const QMap<QString, QStringList>& folder2PileItems);
  bool StartToRearrange();
  int CommandsCnt() const { return m_cmds.size(); }
 private:
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};


#endif // EXTRACTPILEITEMSOUTFOLDER_H
