#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include "SceneInfo.h"
#include <QMap>
#include <QVariantHash>

namespace SceneInfoManager {

#ifdef RUNNING_UNIT_TESTS
inline SceneInfoList& mockScenesInfoList() {
  static SceneInfoList staticSceneInfoList;
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
  static QString GetScnAbsFilePath(const QString& folderPath);
  Counter operator()(const QString& rootPath);  // will update json contents, than generated scn from refreshed jsons
  static int UpdateScnFiles(const QString& rootPath);
  static int ClearScnFiles(const QString& rootPath);
 private:
  Counter UpdateJsonUnderAPath(const QString& path);
};

}  // namespace SceneInfoManager

#endif  // SCENEINFOMANAGER_H
