#ifndef RATEHELPER_H
#define RATEHELPER_H

#include <QString>
#include <QPixmap>

class RateHelper {
 public:
  static bool SetFileRate(const QString& fileAbsPath, int rate);
  static int SetFilesRate(const QStringList& fileAbsPathList, int rate);
  static int SetFileRateRecursively(const QString& folderAbsPath, int rate, bool bOverrideForce = true);

  static bool AdjustFileRate(const QString& fileAbsPath, int delta = 1, int* newRateValue = nullptr);
  static int AdjustFilesRate(const QStringList& fileAbsPathList, int delta = 1);
  static int AdjustFileRateRecursively(const QString& folderAbsPath, int delta = 1);

  static const QPixmap& GetRatePixmap(int rate);

  static bool isClickPointInsideRatingBar(const QPoint& clickPnt, const QRect& vRect);
  static int ratingAtPosition(const QPoint& pos, const QRect& visualRect);

  static constexpr int RATING_BAR_X = 36, RATING_BAR_HEIGHT = 36;

 private:
  static bool SetJsonRateValueCore(const QString& jsonPath, int newRateVal, bool bOverrideForce);
  static bool AdjustJsonRateValueCore(const QString& jsonPath, int delta = 1, int* newRateValue = nullptr);

  static constexpr QColor STD_ORANGE{255, 165, 0, 255};
  static QPixmap GenerateRatePixmap(int r, const int sliceCount, const bool hasBorder = false, QColor color = STD_ORANGE);
};

#endif  // RATEHELPER_H
