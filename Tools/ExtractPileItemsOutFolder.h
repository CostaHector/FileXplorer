#ifndef EXTRACTPILEITEMSOUTFOLDER_H
#define EXTRACTPILEITEMSOUTFOLDER_H

#include <QString>
#include <QMap>

class ExtractPileItemsOutFolder {
 public:
  static bool CanExtractOut(const QStringList& items);
  int operator()(const QString& rootPath);
  int operator()(const QMap<QString, QStringList>& folder2PileItems);
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
