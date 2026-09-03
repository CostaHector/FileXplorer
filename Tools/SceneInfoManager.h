#ifndef SCENEINFOMANAGER_H
#define SCENEINFOMANAGER_H

#include "SceneInfo.h"
#include "JsonOp.h"
#include <QMap>
#include <QVariantHash>

namespace SceneInfoManager {
class ScnMgr {
 public:
  using PATH_2_JSON_DICTS = QMap<QString, QList<QVariantHash>>;
  JsonOp::Counter operator()(const QString& rootPath);  // will update json contents, than generated scn from refreshed jsons
  static int UpdateScnFiles(const QString& rootPath);
  static int ClearScnFiles(const QString& rootPath);
 private:
  JsonOp::Counter UpdateJsonUnderAPath(const QString& path);
};

}  // namespace SceneInfoManager

#endif  // SCENEINFOMANAGER_H
