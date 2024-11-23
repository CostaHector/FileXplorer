#ifndef IMGTHUMBNAILPROCESSER_H
#define IMGTHUMBNAILPROCESSER_H

#include <QStringList>

class ImgThumbnailProcesser {
 public:
  static bool IsImageAnThumbnail(const QString& imgAbsPath);
  static bool IsImageNameLooksLikeThumbnail(const QString& imgBaseName);

  explicit ImgThumbnailProcesser(bool skipIfImgAlreadyExist = true);
  int operator()(const QString& rootPath, int beg = 0, int end = 1);

  QStringList mErrImg;
 private:
  static std::pair<int, int> GetThumbnailDimension(const QString& imgBaseName);
  bool mSkipImageIfAlreadyExist;
  int mExtractImagesCnt{0};
  int mRewriteImagesCnt{0};
};

#endif  // IMGTHUMBNAILPROCESSER_H
