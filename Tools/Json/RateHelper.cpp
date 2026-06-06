#include "RateHelper.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include "ItemsPileCategory.h"
#include "PublicVariable.h"
#include "Logger.h"
#include "TableFields.h"
#include "CastPsonFileHelper.h"
#include <QPainter>
#include <QFile>
#include <QDirIterator>
#include <QRegularExpression>
#include <QSet>

constexpr int RateHelper::RATING_BAR_X, RateHelper::RATING_BAR_HEIGHT;

bool RateHelper::SetJsonRateValueCore(const QString& jsonPath, int newRateVal, bool bOverrideForce) {
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
    int beforeValue = itRate.value().toInt();
    if (afterValue == beforeValue) { // same, skip
      return true;
    }
    if (beforeValue > 0 && !bOverrideForce) {
      return true; // no need override valid Rate(>0)
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

QSet<QString> RateHelper::GetRelatedJsonAbsPaths(const QStringList& paths) {
  QSet<QString> uniqueJsons;
  for (const QString& path : paths) {
    QString jsonPath;
    if (!RateHelper::getJsonPathFromFile(path, jsonPath)) { // no json related
      continue;
    }
    uniqueJsons.insert(jsonPath);
  }
  return uniqueJsons;
}

bool RateHelper::AdjustJsonRateValueCore(const QString& jsonPath, int delta, int* newRateValue) {
  if (delta == 0) {
    return false;
  }
  using namespace JsonHelper;
  using namespace MOVIE_TABLE;
  QVariantHash data = MovieJsonLoader(jsonPath);
  if (!data.contains(ENUM_2_STR(Name))) {
    LOG_D("JSON data[%s] not contains key" ENUM_2_STR(Name), qPrintable(jsonPath));
    return false;
  }

  using namespace PERFORMER_DB_HEADER_KEY;
  auto itRate = data.find(ENUM_2_STR(Rate));
  const int beforeValue{itRate != data.end() ? itRate.value().toInt() : MIN_V};
  const int afterValue{clampRate(beforeValue + delta)};
  if (newRateValue != nullptr) {
    *newRateValue = afterValue;
  }

  if (afterValue == beforeValue) { // already reach border, get clamp
    return true;
  }

  if (itRate != data.end()) {
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

bool RateHelper::SetFileRate(const QString& fileAbsPath, int rate) {
  QString jsonPath;
  if (!getJsonPathFromFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }
  return SetJsonRateValueCore(jsonPath, rate, true);
}

int RateHelper::SetFilesRate(const QStringList& fileAbsPathList, int rate) {
  const QSet<QString> uniqueJsons{RateHelper::GetRelatedJsonAbsPaths(fileAbsPathList)};
  if (uniqueJsons.isEmpty()) {
    return 0;
  }
  int succeedCnt{0}, totalCnt{0};
  for (const QString& jsonAbsPath: uniqueJsons) {
    if (RateHelper::SetJsonRateValueCore(jsonAbsPath, rate, true)) {
      ++succeedCnt;
    }
    ++totalCnt;
  }
  LOG_OE(succeedCnt == totalCnt, "%d/%d json(s) have been updated to rate %d", succeedCnt, totalCnt, rate);
  return succeedCnt;
}

int RateHelper::SetFileRateRecursively(const QString& folderAbsPath, int rate, bool bOverrideForce) {
  QDirIterator it{folderAbsPath, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};

  int succeedCnt{0}, totalCnt{0};
  while (it.hasNext()) {
    if (SetJsonRateValueCore(it.next(), rate, bOverrideForce)) {
      ++succeedCnt;
    }
    ++totalCnt;
  }
  LOG_OE(succeedCnt == totalCnt, "%d/%d json(s) have been updated to rate %d", succeedCnt, totalCnt, rate);

  return succeedCnt;
}

bool RateHelper::AdjustFileRate(const QString& fileAbsPath, int delta, int* newRateValue) {
  if (delta == 0) {
    return false;
  }
  QString jsonPath;
  if (!getJsonPathFromFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }
  return AdjustJsonRateValueCore(jsonPath, delta, newRateValue);
}

int RateHelper::AdjustFilesRate(const QStringList& fileAbsPathList, int delta) {
  const QSet<QString> uniqueJsons{RateHelper::GetRelatedJsonAbsPaths(fileAbsPathList)};
  if (uniqueJsons.isEmpty()) {
    return 0;
  }
  int succeedCnt{0}, totalCnt{0};
  for (const QString& jsonAbsPath: uniqueJsons) {
    if (RateHelper::AdjustJsonRateValueCore(jsonAbsPath, delta)) {
      ++succeedCnt;
    }
    ++totalCnt;
  }
  return succeedCnt;
}

int RateHelper::AdjustFileRateRecursively(const QString& folderAbsPath, int delta) {
  QDirIterator it{folderAbsPath, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};

  int succeedCnt{0}, totalCnt{0};
  while (it.hasNext()) {
    if (AdjustJsonRateValueCore(it.next(), delta)) {
      ++succeedCnt;
    }
    ++totalCnt;
  }
  LOG_OE(succeedCnt == totalCnt, "%d/%d json(s) rate value have been [%d]", succeedCnt, totalCnt, delta);

  return succeedCnt;
}

bool RateHelper::getJsonPathFromFile(const QString& fileAbsPath, QString& jsonPath) {
  jsonPath.clear();

  QString baseName, ext;
  std::tie(baseName, ext) = PathTool::GetBaseNameExt(fileAbsPath);
  const int choppedSize{1 + baseName.size() + ext.size()};
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
  mp.fill(OPAGUE);

  QPainter painter{&mp};
  {
    static constexpr QColor STD_ORANGE{255, 165, 0, 255};
    painter.setPen(STD_ORANGE);
    painter.setBrush(STD_ORANGE);
    painter.drawRect(0, 0, orangeWidth, HEIGHT);
  }

  {
    static constexpr QColor STD_GRAY{0xAF, 0xAF, 0xAF, 168};
    painter.setPen(STD_GRAY);
    painter.drawLine(QLine{20, 0, 20, HEIGHT});
    painter.drawLine(QLine{40, 0, 40, HEIGHT});
    painter.drawLine(QLine{60, 0, 60, HEIGHT});
    painter.drawLine(QLine{80, 0, 80, HEIGHT});
  }

  if (hasBorder) {
    static constexpr QColor STD_BLACK{0, 0, 0, 255};
    painter.setPen(STD_BLACK);
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

bool RateHelper::isClickPointInsideRatingBar(const QPoint& clickPnt, const QRect& visualRect) {
  return visualRect.contains(clickPnt);
}

int RateHelper::ratingAtPosition(const QPoint& pos, const QRect& visualRect) {
  int delta = pos.x() - visualRect.x();
  int nomindator = visualRect.width();
  int rate = RateHelper::MOVIE_RATE_VALUE::MAX_V * delta / nomindator + 1;
  return RateHelper::clampRate(rate);
}
