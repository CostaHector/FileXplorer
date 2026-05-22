#ifndef VIDEO_STORY_BOARD_H
#define VIDEO_STORY_BOARD_H

#include <QStringList>

class VideoStoryboard {
 public:
  static constexpr int SAMPLE_PERIOD_MIN{1};  // [1, +\infty)
  static bool IsDimensionXValid(int dimensionX);
  static bool IsDimensionYValid(int dimensionY);
  static bool IsWidthPixelAllowed(int widthPixel);
  static bool IsSamplePeriodAllowed(int samplePeriod);
  static bool CheckParameters(int dimensionX, int dimensionY, int widthPixel);

  static bool IsImageAnThumbnail(const QString& imgAbsPath);
  static bool IsImageNameLooksLikeThumbnail(const QString& imgBaseName);
  bool RenameVideoStoryBoardCreatedByPotPlayer(const QString& path) const;
  int Create(const QStringList& files, int dimensionX, int dimensionY, int widthPx, const bool isJpg = true) const;

  explicit VideoStoryboard(bool skipIfImgAlreadyExist);
  int ExtractFrames(const QString& rootPath, int beg = 0, int end = 1) const;
  mutable QStringList mErrImg;

 private:
  static std::pair<int, int> GetThumbnailDimension(const QString& imgBaseName);
  bool mSkipImageIfAlreadyExist{true};
  mutable int mExtractImagesCnt{0};
  mutable int mRewriteImagesCnt{0};
};

#endif  // VIDEO_STORY_BOARD_H
