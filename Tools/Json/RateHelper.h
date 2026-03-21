#ifndef RATEHELPER_H
#define RATEHELPER_H

#include <QString>
#include <QPixmap>

class RateHelper {
 public:
  enum MOVIE_RATE_VALUE {
    MIN_V = 0, // initial value, unspecified value
    MAX_V = 10,
    BUTT_V,
  };

  static bool RateMovie(const QString& fileAbsPath, int rate);
  static int RateMovieRecursively(const QString& folderAbsPath, int rate, bool bOverrideForce = true);

  static bool AdjustRateMovie(const QString& fileAbsPath, int delta = 1, int* newRateValue = nullptr);
  static int AdjustRateMovieRecursively(const QString& folderAbsPath, int delta = 1);

  static const QPixmap& GetRatePixmap(int rate);
  static inline int clampRate(int rate) { return std::max(std::min(rate, (int)MAX_V), (int)MIN_V); }

  static bool isClickPointInsideRatingBar(const QPoint& clickPnt, const QRect& vRect);
  static QRect getRatingRect(QRect visualRect);
  static int ratingAtPosition(const QPoint& pos, const QRect& visualRect);

 private:
  static bool RateMovieCore(const QString& jsonPath, int newRateVal, bool bOverrideForce);

  static bool getJsonPathFromFile(const QString& fileAbsPath, QString& jsonPath);
  static QString getBaseNameForImage(const QString& imagePath);

  static QPixmap GenerateRatePixmap(int r, const int sliceCount = MAX_V, const bool hasBorder = false);

  static constexpr int RATING_BAR_HEIGHT = 50;
};

#endif  // RATEHELPER_H
