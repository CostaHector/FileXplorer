#include "RelatedHelper.h"
#include "PathTool.h"
#include "ItemsPileCategory.h"
#include "Logger.h"
#include <QFile>

namespace RelatedHelper {
bool getJsonPathFromFile(QString fileAbsPath, QString& jsonPath) {
  jsonPath.clear();

  if (fileAbsPath.endsWith(PathTool::THUMBNAIL_IMAGE_SUFFIX)) {
    fileAbsPath.chop(sizeof(PathTool::THUMBNAIL_IMAGE_SUFFIX) - 1);
    fileAbsPath += ".json";
  }

  QString baseName, ext;
  std::tie(baseName, ext) = PathTool::GetBaseNameExt(fileAbsPath);
  const int choppedSize{1 + baseName.size() + ext.size()}; // 1 represent for '/' before baseName
  if (fileAbsPath.size() < choppedSize) {
    return false;
  }
  const QString dirPath{fileAbsPath.chopped(choppedSize)};
  static const ItemsPileCategory::T_DOT_EXT_2_TYPE& dotExt2TypeHash = ItemsPileCategory::GetTypeFromDotExtension();
  const ItemsPileCategory::SCENE_COMPONENT_TYPE fileType = dotExt2TypeHash.value(ext.toLower(), ItemsPileCategory::OTHER);

  bool bFindJson{false};

  QString tempJsonPath;
  switch (fileType) {
    case ItemsPileCategory::JSON: {
      tempJsonPath = fileAbsPath;
      break;
    }
    case ItemsPileCategory::VID: {
      tempJsonPath = PathTool::JoinJsonAbsFilePath(dirPath, baseName);
      break;
    }
    case ItemsPileCategory::IMG: {
      // same name.JSON
      tempJsonPath = PathTool::JoinJsonAbsFilePath(dirPath, baseName);
      if (bFindJson = QFile::exists(tempJsonPath)) {
        jsonPath.swap(tempJsonPath);
        break;
      }
      // name without number.JSON
      tempJsonPath = PathTool::JoinJsonAbsFilePath(dirPath, getBaseNameForImage(baseName));
      break;
    }
    case ItemsPileCategory::OTHER: {
      // folder -> folder/folder.json
      tempJsonPath = PathTool::JoinJsonAbsFilePath(dirPath, baseName + "/" + baseName);
      break;
    }
    default: {
      LOG_W("Unsupported file type: %s", qPrintable(fileAbsPath));
      return false;
    }
  }

  if (!bFindJson) {
    if (bFindJson = QFile::exists(tempJsonPath)) {
      jsonPath.swap(tempJsonPath);
    }
  }
  return bFindJson;
}


QSet<QString> GetRelatedJsonAbsPaths(const QStringList& paths) {
  QSet<QString> uniqueJsons;
  for (const QString& path : paths) {
    QString jsonPath;
    if (!getJsonPathFromFile(path, jsonPath)) { // no json related
      continue;
    }
    uniqueJsons.insert(jsonPath);
  }
  return uniqueJsons;
}

QString getBaseNameForImage(const QString& imageBaseName) {
  using namespace ItemsPileCategory;
  const QRegularExpressionMatch match = IMG_PILE_NAME_PATTERN.match(imageBaseName);
  if (match.hasMatch()) {
    return match.captured(1);
  }
  return imageBaseName;
}
}
