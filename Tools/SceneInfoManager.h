#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include <QString>
#include <QList>

struct SCENE_INFO {
  QString name;      // name, key"Name"
  QString imgName;   // img, key"ImgName"
  QString vidName;   // video, key"VidName"
  qint64 vidSize;    // video size, from json file, key"VidSize"
  int rate;          // video rate, from json file, key"Rate"
  QString uploaded;  // from json file, key"Uploaded"
};

typedef QList<SCENE_INFO> SCENES_TYPE;

class SceneInfoManager {
 public:
  static int UpdateJsonImgVidSize(const QString& path);
  static SCENES_TYPE GetScenesFromPath(const QString& path, const bool enableFilter=false, const QString& pattern = {}, SCENES_TYPE* pFiltered = nullptr);

  typedef enum tagSortByKey { NAME = 0, SIZE, RATE, UPLOADED, SORT_BY_KEY_BUTT } SortByKey;
  void sort(const SortByKey sortByKey = NAME, const bool reverse = false);

  void swap(SCENES_TYPE& rhs) { mScenes.swap(rhs); }
  inline int size() { return mScenes.size(); }
  inline int isEmpty() { return mScenes.isEmpty(); }
  const SCENE_INFO& operator[](const int index) const { return mScenes[index]; }

 private:
  SCENES_TYPE mScenes;
};

#endif  // SCENEINFOMANAGER_H
