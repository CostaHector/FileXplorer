#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QVariantHash>

struct SCENE_INFO {
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
  bool operator<(const SCENE_INFO& other) const;
};

typedef QList<SCENE_INFO> SCENE_INFO_LIST;

namespace SceneInfoManager {
SCENE_INFO_LIST ParseAScnFile(const QString& scnFileFullPath, const QString rel);
SCENE_INFO_LIST GetScnsLstFromPath(const QString& path);

#ifdef RUNNING_UNIT_TESTS
inline SCENE_INFO_LIST& mockScenesInfoList() {
  static SCENE_INFO_LIST staticSceneInfoList;
  return staticSceneInfoList;
}
#endif

struct Counter {
  Counter(int jsonUpdatedCnt = 0, int jsonUsedCnt = 0, int vidNameKeyFieldUpdatedCnt = 0, int imgNameKeyFieldUpdatedCnt = 0)
      : m_jsonUpdatedCnt{jsonUpdatedCnt},
        m_jsonUsedCnt{jsonUsedCnt},
        m_VidNameKeyFieldUpdatedCnt{vidNameKeyFieldUpdatedCnt},
        m_ImgNameKeyFieldUpdatedCnt{imgNameKeyFieldUpdatedCnt} {}
  int m_jsonUpdatedCnt;
  int m_jsonUsedCnt;
  int m_VidNameKeyFieldUpdatedCnt;
  int m_ImgNameKeyFieldUpdatedCnt;
  Counter& operator+=(const Counter& rhs) {
    m_jsonUpdatedCnt += rhs.m_jsonUpdatedCnt;
    m_jsonUsedCnt += rhs.m_jsonUsedCnt;
    m_VidNameKeyFieldUpdatedCnt += rhs.m_VidNameKeyFieldUpdatedCnt;
    m_ImgNameKeyFieldUpdatedCnt += rhs.m_ImgNameKeyFieldUpdatedCnt;
    return *this;
  }
  bool operator==(const Counter& rhs) const {
    return m_jsonUpdatedCnt == rhs.m_jsonUpdatedCnt                           //
           && m_jsonUsedCnt == rhs.m_jsonUsedCnt                              //
           && m_VidNameKeyFieldUpdatedCnt == rhs.m_VidNameKeyFieldUpdatedCnt  //
           && m_ImgNameKeyFieldUpdatedCnt == rhs.m_ImgNameKeyFieldUpdatedCnt;
  }
  bool isEmpty() const { return m_jsonUpdatedCnt == 0 && m_jsonUsedCnt == 0; }
};

class ScnMgr {
 public:
  using PATH_2_JSON_DICTS = QMap<QString, QList<QVariantHash>>;
  Counter operator()(const QString& rootPath);  // will update json contents, than generated scn from refreshed jsons
  int WriteDictIntoScnFiles();
 private:
  Counter UpdateJsonUnderAPath(const QString& path);
#ifdef RUNNING_UNIT_TESTS
  void mockJsonDictForTest(const PATH_2_JSON_DICTS& newValue) {
    m_jsonsDicts = newValue;
  }
#endif
  PATH_2_JSON_DICTS m_jsonsDicts;  // relativePathToJsonFile -> Jsons
};

}  // namespace SceneInfoManager

#endif  // SCENEINFOMANAGER_H
