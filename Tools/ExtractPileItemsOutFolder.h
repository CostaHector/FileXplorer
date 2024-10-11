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

class ScenesMixed {
 public:
  int operator()(const QString& path);
  int operator()(const QStringList& files);
  const QString& GetFirstImg(const QString& baseName) const;
  const QString& GetFirstVid(const QString& baseName) const;
  QMap<QString, QStringList> m_img2Name;  // images baseName, extension with prefix dot
  QMap<QString, QStringList> m_vid2Name;
  QMap<QString, QString> m_json2Name;
};

#endif // EXTRACTPILEITEMSOUTFOLDER_H
