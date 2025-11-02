#include "RateHelper.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include "ItemsPileCategory.h"
#include "PublicVariable.h"
#include "Logger.h"
#include "TableFields.h"
#include <QPainter>
#include <QFile>
#include <QDirIterator>
#include <QRegularExpression>

bool RateHelper::RateMovieCore(const QString& jsonPath, int newRateVal, bool bOverrideForce) {
  using namespace JsonHelper;
  using namespace MOVIE_TABLE;
  QVariantHash data = MovieJsonLoader(jsonPath);
  if (!data.contains(ENUM_2_STR(Name))) {
    LOG_D("JSON data[%s] not contains key" ENUM_2_STR(Name), qPrintable(jsonPath));
    return false;
  }

  const int afterValue = clampRate(newRateVal);

  using namespace PERFORMER_DB_HEADER_KEY;
  auto itRate = data.find(ENUM_2_STR(Rate));
  if (itRate != data.cend()) { // Rate already exist
    if (!bOverrideForce) {
      return true; // no need override Rate
    }

    int beforeValue = itRate.value().toInt();
    if (afterValue == beforeValue) { // same, skip
      return true;
    }
    itRate->setValue(afterValue);
  } else {
    data[ENUM_2_STR(Rate)] = afterValue;
  }

  if (!DumpJsonDict(data, jsonPath)) {
    LOG_E("Failed to save JSON data to: %s", qPrintable(jsonPath));
    return false;
  }
  return true;
}

bool RateHelper::RateMovie(const QString& fileAbsPath, int rate) {
  QString jsonPath;
  if (!getJsonPathForFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }
  return RateMovieCore(jsonPath, rate, true);
}

int RateHelper::RateMovieRecursively(const QString& folderAbsPath, int rate, bool bOverrideForce) {
  QDirIterator it{folderAbsPath, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  int succeedCnt{0}, totalCnt{0};
  while (it.hasNext()) {
    succeedCnt += RateMovieCore(it.next(), rate, bOverrideForce);
    ++totalCnt;
  }
  LOG_OE(succeedCnt == totalCnt, "%d/%d json(s) have been updated to rate %d", succeedCnt, totalCnt, rate);

  return succeedCnt;
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
  static const T_DOT_EXT_2_TYPE& dotExt2TypeHash = GetTypeFromDotExtension();
  SCENE_COMPONENT_TYPE fileType = dotExt2TypeHash.value(ext.toLower(), OTHER);

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
    case OTHER: {
      // folder -> folder/folder.json
      tempJsonPath = JoinDirAndBasename(baseName + "/" + baseName);
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

QPixmap RateHelper::GenerateRatePixmap(int r, const int sliceCount, const bool hasBorder) {
  if (r < 0 || r > sliceCount) {
    LOG_D("rate[%d] out bound", r);
    return {};
  }
  static constexpr int WIDTH = 100, HEIGHT = (int) (WIDTH * 0.618);
  QPixmap mp{WIDTH, HEIGHT};
  int orangeWidth = WIDTH * r / sliceCount;
  static constexpr QColor OPAGUE{0, 0, 0, 0};
  static constexpr QColor STD_ORANGE{255, 165, 0, 255};
  mp.fill(OPAGUE); // opague
  QPainter painter{&mp};
  painter.setPen(STD_ORANGE); // standard orange
  painter.setBrush(STD_ORANGE);
  painter.drawRect(0, 0, orangeWidth, HEIGHT);
  if (hasBorder) {
    painter.setPen(QColor{0, 0, 0, 255}); // standard black
    painter.setBrush(OPAGUE);
    painter.drawRect(0, 0, WIDTH - 1, HEIGHT - 1);
  }
  painter.end();
  return mp;
}

const QPixmap& RateHelper::GetRatePixmap(int rate) {
  static_assert(MIN_V == 0, "Minumum rate value should be 0");
  static_assert(MAX_V == 10, "Maximum rate value should be 10");
  static const QPixmap SCORE_BOARD[BUTT_V] //
      {
          GenerateRatePixmap(0, MAX_V),  //
          GenerateRatePixmap(1, MAX_V),  //
          GenerateRatePixmap(2, MAX_V),  //
          GenerateRatePixmap(3, MAX_V),  //
          GenerateRatePixmap(4, MAX_V),  //
          GenerateRatePixmap(5, MAX_V),  //
          GenerateRatePixmap(6, MAX_V),  //
          GenerateRatePixmap(7, MAX_V),  //
          GenerateRatePixmap(8, MAX_V),  //
          GenerateRatePixmap(9, MAX_V),  //
          GenerateRatePixmap(10, MAX_V), //
      };
  return SCORE_BOARD[clampRate(rate)];
}
