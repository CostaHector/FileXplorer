#ifndef RATEHELPER_H
#define RATEHELPER_H

#include <QString>
#include <QPixmap>

class RateHelper {
public:
  enum MOVIE_RATE_VALUE {
    MIN_V = 0,
    MAX_V = 10,
    BUTT_V,
  };

  static bool RateMovie(const QString& fileAbsPath, int rate);
  static int RateMovieRecursively(const QString& folderAbsPath, int rate);
  static const QPixmap& GetRatePixmap(int rate);

private:
  static inline int clampRate(int rate) { return std::max(std::min(rate, (int) MAX_V), (int) MIN_V); }

  static bool RateMovieCore(const QString& jsonPath, int rate);

  static bool getJsonPathForFile(const QString& fileAbsPath, QString& jsonPath);
  static QString getBaseNameForImage(const QString& imagePath);

  static QPixmap GenerateRatePixmap(int r, const int sliceCount = MAX_V, const bool hasBorder = false);
};

#endif // RATEHELPER_H
