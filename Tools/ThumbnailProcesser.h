#ifndef IMGTHUMBNAILPROCESSER_H
#define IMGTHUMBNAILPROCESSER_H

#include <QStringList>

class ThumbnailProcesser {
 public:
  static const QList<int> mAllowedPixelList;
  static constexpr int SAMPLE_PERIOD_MIN{1}, SAMPLE_PERIOD_MAX{300 + 1};  // [1, 300)
  static bool IsDimensionXValid(int dimensionX);
  static bool IsDimensionYValid(int dimensionY);
  static bool IsWidthPixelAllowed(int widthPixel);
  static bool IsSamplePeriodAllowed(int samplePeriod);

  static bool IsImageAnThumbnail(const QString& imgAbsPath);
  static bool IsImageNameLooksLikeThumbnail(const QString& imgBaseName);
  static int CreateThumbnailImages(const QStringList& files, int dimensionX, int dimensionY, int widthPx, const int timePeriod = 20, const bool isJpg = true);

  explicit ThumbnailProcesser(bool skipIfImgAlreadyExist = true);
  int operator()(const QString& rootPath, int beg = 0, int end = 1);
  QStringList mErrImg;

 private:
  static std::pair<int, int> GetThumbnailDimension(const QString& imgBaseName);
  bool mSkipImageIfAlreadyExist;
  int mExtractImagesCnt{0};
  int mRewriteImagesCnt{0};
};

#endif  // IMGTHUMBNAILPROCESSER_H
