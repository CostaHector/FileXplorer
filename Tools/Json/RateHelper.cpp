#include "RateHelper.h"
#include "JsonHelper.h"
#include "PublicVariable.h"
#include "Logger.h"
#include "JsonModelField.h"
#include "JsonFieldBoundary.h"
#include "RelatedHelper.h"
#include <QPainter>
#include <QFile>
#include <QDirIterator>
#include <QRegularExpression>
#include <QSet>

constexpr int RateHelper::RATING_BAR_X, RateHelper::RATING_BAR_HEIGHT;

bool RateHelper::SetJsonRateValueCore(const QString& jsonPath, int newRateVal, bool bOverrideForce) {
  using namespace JsonModelField;
  QVariantHash data = JsonHelper::MovieJsonLoader(jsonPath);
  if (!data.contains(ENUM_2_STR(Name))) {
    LOG_D("JSON data[%s] not contains key" ENUM_2_STR(Name), qPrintable(jsonPath));
    return false;
  }

  const int afterValue = JsonFieldBoundary::clampRate(newRateVal);

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

  if (!JsonHelper::DumpJsonDict(data, jsonPath)) {
    LOG_E("Failed to save JSON data to: %s", qPrintable(jsonPath));
    return false;
  }
  return true;
}

bool RateHelper::AdjustJsonRateValueCore(const QString& jsonPath, int delta, int* newRateValue) {
  if (delta == 0) {
    return false;
  }
  using namespace JsonModelField;
  QVariantHash data = JsonHelper::MovieJsonLoader(jsonPath);
  if (!data.contains(ENUM_2_STR(Name))) {
    LOG_D("JSON data[%s] not contains key" ENUM_2_STR(Name), qPrintable(jsonPath));
    return false;
  }

  auto itRate = data.find(ENUM_2_STR(Rate));
  const int beforeValue{itRate != data.end() ? itRate.value().toInt() : JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V};
  const int afterValue{JsonFieldBoundary::clampRate(beforeValue + delta)};
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

  if (!JsonHelper::DumpJsonDict(data, jsonPath)) {
    LOG_E("Failed to save JSON data to: %s", qPrintable(jsonPath));
    return false;
  }
  return true;
}

bool RateHelper::SetFileRate(const QString& fileAbsPath, int rate) {
  QString jsonPath;
  if (!RelatedHelper::getJsonPathFromFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }
  return SetJsonRateValueCore(jsonPath, rate, true);
}

int RateHelper::SetFilesRate(const QStringList& fileAbsPathList, int rate) {
  const QSet<QString> uniqueJsons{RelatedHelper::GetRelatedJsonAbsPaths(fileAbsPathList)};
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
  if (!RelatedHelper::getJsonPathFromFile(fileAbsPath, jsonPath)) {
    LOG_W("JSON file not found by[%s]", qPrintable(fileAbsPath));
    return false;
  }
  return AdjustJsonRateValueCore(jsonPath, delta, newRateValue);
}

int RateHelper::AdjustFilesRate(const QStringList& fileAbsPathList, int delta) {
  const QSet<QString> uniqueJsons{RelatedHelper::GetRelatedJsonAbsPaths(fileAbsPathList)};
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
  static const QPixmap SCORE_BOARD[JsonFieldBoundary::RATE_BUTT_V] //
      {
          GenerateRatePixmap(0,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(1,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(2,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(3,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(4,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(5,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(6,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(7,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(8,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(9,  JsonFieldBoundary::RATE_MAX_V),  //
          GenerateRatePixmap(10, JsonFieldBoundary::RATE_MAX_V), //
      };
  return SCORE_BOARD[JsonFieldBoundary::clampRate(rate)];
}

bool RateHelper::isClickPointInsideRatingBar(const QPoint& clickPnt, const QRect& visualRect) {
  return visualRect.contains(clickPnt);
}

int RateHelper::ratingAtPosition(const QPoint& pos, const QRect& visualRect) {
  int delta = pos.x() - visualRect.x();
  int nomindator = visualRect.width();
  int rate = JsonFieldBoundary::RATE_MAX_V * delta / nomindator + 1;
  return JsonFieldBoundary::clampRate(rate);
}
