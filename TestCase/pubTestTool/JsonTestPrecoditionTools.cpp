#include "JsonTestPrecoditionTools.h"
#include "Logger.h"

SceneInfoManager::ScnMgr::PATH_2_JSON_DICTS JsonTestPrecoditionTools::GetPathJsonDictList(  //
    const QString& name0,
    const QString& scenesUnderRootPath,
    QVariantHash& scene0,  //
    const QString& name1,
    const QString& scenesUnderSubPath,
    QVariantHash& scene1) {  //
  // "The Last Fight", tDir.path(), "Iron Man", tDir.itemPath("Avengers")
  // return in order. i.e., name0 should be ahead of name
  using namespace SceneInfoManager;

  scene0.clear();
  scene0["Name"] = name0;
  scene0["ImgName"] = QStringList{name0 + " 1.jpg", name0 + " 2.jpg"};
  scene0["VidName"] = name0 + ".mp4";
  scene0["Size"] = 1024 * 1024 * 500;
  scene0["Rate"] = 10;  // 10 in 10
  scene0["Uploaded"] = "20221212 13:00:00";
  QList<QVariantHash> scenesUnderRoot{scene0};

  scene1.clear();
  scene1["Name"] = name1;
  scene1["ImgName"] = QStringList{name1 + ".jpg"};
  scene1["VidName"] = name1 + ".mp4";
  scene1["Size"] = 1024 * 1024 * 700;
  scene1["Rate"] = 9;  // 9 in 10
  scene1["Uploaded"] = "20241212 13:00:00";
  QList<QVariantHash> scenesUnderAvengers{scene1};

  ScnMgr::PATH_2_JSON_DICTS testData{
      {scenesUnderRootPath, {scenesUnderRoot}},
      {scenesUnderSubPath, {scenesUnderAvengers}},
  };

  return testData;
}
