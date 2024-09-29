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

  typedef enum tagSortByKey { NAME = 0, SIZE, RATE, UPLOADED, SORT_BY_KEY_BUTT } SortByKey;
  SCENES_TYPE GetScenesFromPath(const QString& path);
  void sort(const SortByKey sortByKey = NAME, const bool reverse = false);

  void swap(SCENES_TYPE& rhs) { mScenes.swap(rhs); }
  inline int size() { return mScenes.size(); }
  inline int isEmpty() { return mScenes.isEmpty(); }
  SCENE_INFO& operator[](const int index) { return mScenes[index]; }

 private:
  SCENES_TYPE mScenes;
};

#endif  // SCENEINFOMANAGER_H
