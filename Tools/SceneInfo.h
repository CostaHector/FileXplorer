#ifndef SCENEINFO_H
#define SCENEINFO_H

#include <QString>
#include <QList>
#include <QDataStream>

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

  enum Role {
    DEF_BEGIN_ROLE = Qt::DisplayRole,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,  // name
    REL_PATH_ROLE = Qt::UserRole + 1,     // rel2scn + name
    VID_SIZE_ROLE,                        // vidSize
    RATE_ROLE,                            // rate
    UPLOADED_ROLE,                        // uploaded
    INVALID_BUTT_ROLE,
  };

  QString GetAbsolutePath(const QString& rootPath) const;
  QString GetFirstImageAbsPath(const QString& rootPath) const;
  QStringList GetImagesAbsPathList(const QString& rootPath) const;
  QString GetVideoAbsPath(const QString& rootPath) const;
  QStringList GetVideosAbsPath(const QString& rootPath) const;
  QString GetJsonAbsPath(const QString& rootPath) const;

  using CompareFunc = bool (*)(const SceneInfo&, const SceneInfo&);
  static CompareFunc getCompareFunc(SceneInfo::Role dim);

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

  static Role GetInitialSortRole();
  static void SaveInitialSortRole(Role sortRole);
  static bool GetInitialSortOrderReverse();
  static void SaveSortOrderReverse(bool bReverse);
  static bool GetInitialDisableImageDecoration();
  static void SaveDisableImageDecoration(bool bDisable);

  static constexpr int SORT_COLUMN = 0;
  static constexpr Role DEF_SORT_ROLE = REL_PATH_ROLE;
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
