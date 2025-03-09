#ifndef SCENES_MIXED_H
#define SCENES_MIXED_H

#include <QString>
#include <QMap>

class ScenesMixed {
 public:
  QMap<QString, QStringList> operator()(const QString& path);
  QMap<QString, QStringList> operator()(const QStringList& files);
  const QString& GetFirstImg(const QString& baseName) const;
  const QStringList& GetAllImgs(const QString& baseName) const;
  const QString& GetFirstVid(const QString& baseName) const;

  QMap<QString, QStringList> m_img2Name;  // images baseName, extension with prefix dot
  QMap<QString, QStringList> m_vid2Name;
  QMap<QString, QString> m_json2Name;
 private:
  bool NeedCombine2Folder(const QString& folderNameLhs, const QString& folderNameRhs);
};

#endif
