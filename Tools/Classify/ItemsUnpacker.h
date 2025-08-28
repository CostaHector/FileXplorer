#ifndef ITEMSUNPACKER_H
#define ITEMSUNPACKER_H

#include "FileOperatorPub.h"
#include <QString>
#include <QMap>

class ItemsUnpacker {
 public:
  static QMap<QString, QStringList> GetFolder2ItemsMapByCurPath(const QString& path);

  static bool CanExtractOut(const QStringList& items);
  int operator()(const QString& path);
  int operator()(const QString& path,                                 //
                 const QMap<QString, QStringList>& folder2PileItems,  //
                 const QStringList& alreadyExistItems);
  bool StartToRearrange();
  int CommandsCnt() const { return m_cmds.size(); }

 private:
  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};

#endif  // ITEMSUNPACKER_H
