#ifndef RATEHELPER_H
#define RATEHELPER_H

#include <QString>

class RateHelper {
public:
  static bool RateMovie(const QString& fileAbsPath, int rate);

private:
  static bool getJsonPathForFile(const QString& fileAbsPath, QString& jsonPath);
  static QString getBaseNameForImage(const QString& imagePath);
  static constexpr int MOVIE_MIN_RATE = 0, MOVIE_MAX_RATE = 10;
};


#endif // RATEHELPER_H
