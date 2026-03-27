#ifndef SCENEINFO_H
#define SCENEINFO_H

#include <QString>
#include <QList>
#include <QDataStream>
#include "ScenePageNaviHelper.h"

struct SceneInfo {
  friend QDataStream& operator<<(QDataStream& os, const SceneInfo& item);
  friend QDataStream& operator>>(QDataStream& is, SceneInfo& item);
  static SceneInfo fromJsonVariantHash(const QVariantHash& varHash);
  QString rel2scn;   // jsonFullPath = mRootPath + relative2scnFile + jsonFileName, rel2scn can be '/' or '/any thing/'
  QString name;      // name, key"Name" from json file baseName
  QStringList imgs;  // img, key"ImgName"
  QString vidName;   // video, key"VidName"
  qint64 vidSize;    // video size, from json file, key"Size"
  int rate;          // video rate, from json file, key"Rate"
  QString uploaded;  // from json file, key"Uploaded"

  QString GetAbsolutePath(const QString& rootPath) const;
  QString GetFirstImageAbsPath(const QString& rootPath) const;
  QStringList GetImagesAbsPathList(const QString& rootPath) const;
  QString GetVideoAbsPath(const QString& rootPath) const;
  QStringList GetVideosAbsPath(const QString& rootPath) const;
  QString GetJsonAbsPath(const QString& rootPath) const;

  using CompareFunc = bool(*)(const SceneInfo&, const SceneInfo&);
  static CompareFunc getCompareFunc(SceneSortOrderHelper::SortDimE dim);

  bool operator<(const SceneInfo& other) const;
  bool operator==(const SceneInfo& rhs) const;

  static bool less(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanName(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanVidSize(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanRate(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanUploaded(const SceneInfo& self, const SceneInfo& other);

  bool GetNameFromStream(QDataStream& stream);
  bool DeviateStreamFromNameToRateAndOverrideRate(QDataStream& stream, int newRate);

  static constexpr quint32 MAGIC_NUMBER = 0x4C4D5343;  // "LMSC" = "Local Media Scene Cache"
  static constexpr quint16 CURRENT_VERSION = 1;
  static constexpr quint16 MIN_SUPPORTED_VERSION = 1;
  using ELEMENT_COUNT_TYPE = int;
};
typedef QList<SceneInfo> SceneInfoList;
inline QDataStream& operator<<(QDataStream& os, const SceneInfo& item) {
  return os << item.rel2scn << item.name << item.imgs << item.vidName << item.vidSize << item.rate << item.uploaded;
}

inline QDataStream& operator>>(QDataStream& is, SceneInfo& item) {
  return is >> item.rel2scn >> item.name >> item.imgs >> item.vidName >> item.vidSize >> item.rate >> item.uploaded;
}

namespace SceneHelper {
SceneInfoList ParseAScnFile(const QString& scnFileFullPath, const QString& rel);
SceneInfoList GetScnsLstFromPath(const QString& path);
bool SaveScenesListToBinaryFile(const QString& scnAbsFilePath, const SceneInfoList& scenes);

bool UpdateNameWithNewRate(const QString& scnFilePath, const QString& specifiedName, int newRate);
}  // namespace SceneHelper

#endif  // SCENEINFO_H
