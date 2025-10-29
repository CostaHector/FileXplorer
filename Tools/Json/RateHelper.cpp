#include "RateHelper.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include "ItemsPileCategory.h"
#include "Logger.h"
#include <QFileInfo>
#include <QRegularExpression>

constexpr int RateHelper::MOVIE_MIN_RATE, RateHelper::MOVIE_MAX_RATE;

bool RateHelper::RateMovie(const QString& fileAbsPath, int rate) {
  QString jsonPath;
  if (!getJsonPathForFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }

  using namespace JsonHelper;
  QVariantHash data = MovieJsonLoader(jsonPath);
  if (!data.contains("Name")) {
    LOG_D("JSON data[%s] not contains key `Name`", qPrintable(jsonPath));
    return false;
  }

  int beforeValue = data.value("Rate", -1).toInt();
  int afterValue = std::max(std::min(rate, MOVIE_MAX_RATE), MOVIE_MIN_RATE);
  if (afterValue == beforeValue) {
    return true;
  }
  data["Rate"] = afterValue;

  if (!DumpJsonDict(data, jsonPath)) {
    LOG_E("Failed to save JSON data to: %s", qPrintable(jsonPath));
    return false;
  }
  return true;
}

bool RateHelper::getJsonPathForFile(const QString& fileAbsPath, QString& jsonPath) {
  jsonPath.clear();

  QString baseName, ext;
  std::tie(baseName, ext) = PathTool::GetBaseNameExt(fileAbsPath);
  const QString dirPath{fileAbsPath.chopped(1 + baseName.size() + ext.size())};
  const auto JoinDirAndBasename = [dirPath](const QString& baseName) -> QString {
    QString jsonPath;
    jsonPath.reserve(dirPath.size() + 1 + baseName.size() + 5);
    jsonPath += dirPath;
    jsonPath += "/";
    jsonPath += baseName;
    jsonPath += ".json";
    return jsonPath;
  };

  using namespace ItemsPileCategory;
  SCENE_COMPONENT_TYPE fileType = DOT_EXT_2_TYPE.value(ext.toLower(), OTHER);

  bool bFindJson{false};
  QString tempJsonPath;

  switch (fileType) {
    case JSON: {
      tempJsonPath = fileAbsPath;
      break;
    }
    case VID: {
      tempJsonPath = JoinDirAndBasename(baseName);
      break;
    }
    case IMG: {
      // same name.JSON
      tempJsonPath = JoinDirAndBasename(baseName);
      if (bFindJson = QFile::exists(tempJsonPath)) {
        jsonPath.swap(tempJsonPath);
        break;
      }
      // name without number.JSON
      tempJsonPath = JoinDirAndBasename(getBaseNameForImage(baseName));
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

QString RateHelper::getBaseNameForImage(const QString& imageBaseName) {
  using namespace ItemsPileCategory;
  const QRegularExpressionMatch match = IMG_PILE_NAME_PATTERN.match(imageBaseName);
  if (match.hasMatch()) {
    return match.captured(1);
  }
  return imageBaseName;
}
