#ifndef IMGTHUMBNAILPROCESSER_H
#define IMGTHUMBNAILPROCESSER_H

#include <QStringList>

class ThumbnailProcesser {
 public:
  static bool IsImageAnThumbnail(const QString& imgAbsPath);
  static bool IsImageNameLooksLikeThumbnail(const QString& imgBaseName);
  static int CreateThumbnailImages(int dimensionX, int dimensionY, int widthPx, const QStringList& files, const bool isJpg=true, const int timePeriod = 20);

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
