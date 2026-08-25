#ifndef SCENES_MIXED_H
#define SCENES_MIXED_H

#include <QString>
#include <QMap>
#include <QSet>

class ScenesMixed {
 public:
  using GROUP_MAP_TYPE = QMap<QString, QStringList>;
  explicit ScenesMixed() = default;
  GROUP_MAP_TYPE operator()(const QString& path);
  GROUP_MAP_TYPE operator()(const QStringList& files);
  const QStringList& GetAllImgs(const QString& baseName) const;
  const QString& GetFirstVid(const QString& baseName) const;

  GROUP_MAP_TYPE m_img2Name;  // images baseName, extension with prefix dot
  GROUP_MAP_TYPE m_vid2Name;
  QSet<QString> m_json2Name;
 private:
  bool NeedCombine2Folder(const QString& folderNameLhs, const QString& folderNameRhs) const;
};

#endif
