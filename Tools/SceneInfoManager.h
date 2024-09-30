#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include <QString>
#include <QList>

struct SCENE_INFO {
  QString rel2scn;  // jsonFullPath = mRootPath + rel2scn + jsonFileName, rel2scn can be '/' or '/any thing/'
  QString name;      // name, key"Name"
  QString imgName;   // img, key"ImgName"
  QString vidName;   // video, key"VidName"
  qint64 vidSize;    // video size, from json file, key"VidSize"
  int rate;          // video rate, from json file, key"Rate"
  QString uploaded;  // from json file, key"Uploaded"
};

typedef QList<SCENE_INFO> SCENES_TYPE;

namespace SceneInfoManager {
typedef enum class tagSortByKey { NAME = 0, SIZE, RATE, UPLOADED, BUTT } SortByKey;

int UpdateJsonImgVidSize(const QString& path);
SCENES_TYPE ScnFileParser(const QString& scnFileFullPath, const QString rel,
                          bool enableFilter, const QString& pattern, SCENES_TYPE* pFilterd = nullptr);
int GenerateScnFiles(const QString& path);

SCENES_TYPE GetScenesFromPath(const QString& path, const bool enableFilter = false, const QString& pattern = {}, SCENES_TYPE* pFiltered = nullptr);
SCENES_TYPE& sort(SCENES_TYPE& scenes, SortByKey sortByKey = tagSortByKey::NAME, const bool reverse = false);
}  // namespace SceneInfoManager

#endif  // SCENEINFOMANAGER_H
