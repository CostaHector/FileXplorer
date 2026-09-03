#ifndef SCENEINFO_H
#define SCENEINFO_H

#include <QString>
#include <QList>
#include <QDataStream>

struct SceneInfo {
  friend QDataStream& operator<<(QDataStream& os, const SceneInfo& item);
  friend QDataStream& operator>>(QDataStream& is, SceneInfo& item);
  static SceneInfo fromJsonVariantHash(const QVariantHash& varHash, const QString& _jsonFileName);

  SceneInfo() = default;
  explicit SceneInfo(const QString& _name, const QStringList& _imgs, const QString& _vidName, const qint64& _vidSize, int _rate, const QString& _jsonFileName)//
    : name{_name}, imgs{_imgs}, vidName{_vidName}, vidSize{_vidSize}, rate{_rate}, jsonFileName{_jsonFileName} {}
#ifdef RUNNING_UNIT_TESTS
  explicit SceneInfo(const QString& _rel2scn, const QString& _name, const QStringList& _imgs, const QString& _vidName, const qint64& _vidSize, int _rate, const QString& _jsonFileName)//
    : SceneInfo{_name, _imgs, _vidName, _vidSize, _rate, _jsonFileName} { InitRel2Scn(_rel2scn); }
#endif
  /* below member are read out from json file contents */
  QString name;      // name, value of key "Name". usually it is the baseName of json file
  QStringList imgs;  // img, value of key "ImgName"
  QString vidName;   // video, value of key "VidName"
  qint64 vidSize;    // video size, value of key "Size"
  int rate;          // video rate, value of key "Rate"
  QString jsonFileName;
  /* above member are read out from json file contents */

  // depend which folder .scn file is located in. only init me when parse an scn file or in test. Don't serialize or deserialize me
  QString rel2scn;   // jsonFullPath = mRootPath + relative2scnFile + jsonFileName, rel2scn can be '/' or '/any thing/'
  void InitRel2Scn(const QString& _rel2scn) { rel2scn = _rel2scn; }

  enum Role {
    DEF_BEGIN_ROLE = Qt::DisplayRole,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,  // name
    REL_PATH_ROLE = Qt::UserRole + 1,     // rel2scn + name
    VID_SIZE_ROLE,                        // vidSize
    RATE_ROLE,                            // rate
    INVALID_BUTT_ROLE,
  };
  static QString GetSceneFullPathStatic(const QString& folderPath);
  QString GetSceneFullPath(const QString& rootPath) const;
  QString GetAbsolutePath(const QString& rootPath) const;
  QString GetThumbnailImageAbsPath(const QString& rootPath) const;
  QString GetFirstImageAbsPath(const QString& rootPath) const;
  QStringList GetImagesAbsPathList(const QString& rootPath) const;
  QString GetVideoFullPath(const QString& rootPath) const;
  QStringList GetVideosFullPathWithFallback(const QString& rootPath) const;
  QString GetJsonAbsPath(const QString& rootPath) const;

  using CompareFunc = bool (*)(const SceneInfo&, const SceneInfo&);
  static CompareFunc getCompareFunc(SceneInfo::Role dim);

  bool operator<(const SceneInfo& other) const;
  bool operator==(const SceneInfo& rhs) const;

  static bool less(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanName(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanVidSize(const SceneInfo& self, const SceneInfo& other);
  static bool lessThanRate(const SceneInfo& self, const SceneInfo& other);

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
  static bool GetIncludeScnInSubdirectories();
  static void SaveIncludeScnInSubdirectories(bool bIncludeScnInSubdirectory);

  static constexpr int SORT_COLUMN = 0;
  static constexpr Role DEF_SORT_ROLE = REL_PATH_ROLE;
};
typedef QList<SceneInfo> SceneInfoList;
inline QDataStream& operator<<(QDataStream& os, const SceneInfo& item) {
  return os << item.name << item.imgs << item.vidName << item.vidSize << item.rate << item.jsonFileName;
}

inline QDataStream& operator>>(QDataStream& is, SceneInfo& item) {
  return is >> item.name >> item.imgs >> item.vidName >> item.vidSize >> item.rate >> item.jsonFileName;
}

namespace SceneHelper {
SceneInfoList ParseAScnFile(const QString& scnFileFullPath, const QString& rel);
SceneInfoList GetScnsLstFromPath(const QString& path, bool bSubdirectories = true);
bool SaveScenesListToBinaryFile(const QString& scnAbsFilePath, const SceneInfoList& scenes);

bool UpdateNameWithNewRate(const QString& scnFilePath, const QString& specifiedName, int newRate);
}  // namespace SceneHelper

#endif  // SCENEINFO_H
