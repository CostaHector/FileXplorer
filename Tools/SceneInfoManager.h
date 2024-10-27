#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QVariantHash>

struct SCENE_INFO {
  QString rel2scn;   // jsonFullPath = mRootPath + rel2scn + jsonFileName, rel2scn can be '/' or '/any thing/'
  QString name;      // name, key"Name"
  QString imgName;   // img, key"ImgName"
  QString vidName;   // video, key"VidName"
  qint64 vidSize;    // video size, from json file, key"VidSize"
  int rate;          // video rate, from json file, key"Rate"
  QString uploaded;  // from json file, key"Uploaded"
};

typedef QList<SCENE_INFO> SCENES_TYPE;

namespace SceneInfoManager {
enum class SceneSortOption : char { NAME = 0, SIZE, RATE, UPLOADED, BUTT };
SceneSortOption GetSortOptionFromStr(const QString& sortOption);
SCENES_TYPE ScnFileParser(const QString& scnFileFullPath,
                          const QString rel,
                          bool enableFilter,
                          const QString& pattern,
                          SCENES_TYPE* pFilterd = nullptr);

// json file will not updated, read json then generate scn file directly
std::pair<QString, int> GetScnFileContents(const QStringList& jsonNames, const QList<QVariantHash>& jsonDicts);
bool GenerateAScnFile(const QString& aPath);
int GenerateScnFilesDirectly(const QString& rootPath);

SCENES_TYPE GetScenesFromPath(const QString& path, const bool enableFilter = false, const QString& pattern = {}, SCENES_TYPE* pFiltered = nullptr);

SCENES_TYPE& sort(SCENES_TYPE& scenes, SceneSortOption sortByKey = SceneSortOption::NAME, const bool reverse = false);
}  // namespace SceneInfoManager

class JsonDataRefresher {
 public:
  int UpdateAFolderItself(const QString& path);
  // call operator() to refresh json, than generated scn from refreshed jsons
  int operator()(const QString& rootPath);
  int GenerateScnFiles();
  QMap<QString, QList<QVariantHash>> m_jsonsDicts; // relativePathToJsonFile -> Jsons
 private:
  int m_updatedJsonFilesCnt = 0, m_usefullJsonCnt = 0;
};



#endif  // SCENEINFOMANAGER_H
