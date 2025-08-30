#include "ThumbnailProcesser.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include <QRegularExpression>
#include <utility>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QSet>

const QList<int> ThumbnailProcesser::mAllowedPixelList{360, 480, 720, 1080};
constexpr int ThumbnailProcesser::SAMPLE_PERIOD_MIN, ThumbnailProcesser::SAMPLE_PERIOD_MAX;  // [1, 300)

bool ThumbnailProcesser::IsDimensionXValid(int dimensionX) {
  static constexpr int DIMENSION_X_MIN{1}, DIMENSION_X_MAX{10+1};  // [1, 10+1)
  return DIMENSION_X_MIN <= dimensionX && dimensionX <= DIMENSION_X_MAX;
}

bool ThumbnailProcesser::IsDimensionYValid(int dimensionY) {
  static constexpr int DIMENSION_Y_MIN{1}, DIMENSION_Y_MAX{10+1};  // [1, 10+1)
  return DIMENSION_Y_MIN <= dimensionY && dimensionY <= DIMENSION_Y_MAX;
}

bool ThumbnailProcesser::IsWidthPixelAllowed(int widthPixel) {
  static const QSet<int> mAllowedPixelSet{mAllowedPixelList.begin(), mAllowedPixelList.cend()};
  return mAllowedPixelSet.contains(widthPixel);
}

bool ThumbnailProcesser::IsSamplePeriodAllowed(int samplePeriod) {
  return SAMPLE_PERIOD_MIN <= samplePeriod && samplePeriod < SAMPLE_PERIOD_MAX;
}

bool ThumbnailProcesser::CheckParameters(int dimensionX, int dimensionY, int widthPixel) {
  if (!IsDimensionXValid(dimensionX)) {
    LOG_INFO_P("Dimension of row invalid", "%d", dimensionX);
    return false;
  }
  if (!IsDimensionYValid(dimensionY)) {
    LOG_INFO_P("Dimension of column invalid", "%d", dimensionY);
    return false;
  }
  if (!IsWidthPixelAllowed(widthPixel)) {
    LOG_INFO_P("images width invalid", "%d", widthPixel);
    return false;
  }
  const int samplePeriod = Configuration().value(MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.name, MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.v).toInt();
  if (!IsSamplePeriodAllowed(samplePeriod)) {
    LOG_INFO_P("Sample period not allowed", "%d", samplePeriod);
    return false;
  }
  return true;
}

ThumbnailProcesser::ThumbnailProcesser(bool skipIfImgAlreadyExist)  //
    : mSkipImageIfAlreadyExist{skipIfImgAlreadyExist}               //
{}

bool ThumbnailProcesser::IsImageAnThumbnail(const QString& imgAbsPath) {
  QString imgBaseName;
  QString ext;
  std::tie(imgBaseName, ext) = PathTool::GetBaseNameExt(imgAbsPath);
  if (!TYPE_FILTER::IMAGE_TYPE_SET.contains("*" + ext)) {  // not an image
    return false;
  }
  if (!IsImageNameLooksLikeThumbnail(imgBaseName)) {  // name not like
    return false;
  }
  QFileInfo fi{imgAbsPath};
  if (!fi.isFile()) {
    return false;
  }
  static constexpr int IMG_LARGEST_SIZE = 5 * 1024 * 1024;  // 5MiB
  if (fi.size() > IMG_LARGEST_SIZE) {                       // images too large
    return false;
  }
  return true;
}

bool ThumbnailProcesser::IsImageNameLooksLikeThumbnail(const QString& imgBaseName) {
  static const QRegularExpression THUMBNAIL_TWODIGIT_PATTERN{R"( \d\d$)"};
  QRegularExpressionMatch ret;
  if (!(ret = THUMBNAIL_TWODIGIT_PATTERN.match(imgBaseName)).hasMatch()) {
    return false;
  }
  // " ab" <==> row-by-column
  int row = -1;
  int column = -1;
  std::tie(row, column) = GetThumbnailDimension(imgBaseName);
  if (!IsDimensionXValid(row) || !IsDimensionXValid(column)) {  // 0x, x0, row>=9 or column>=9
    return false;
  }
  if (row == 1) {  // 1x -> false
    return false;
  }
  if (column == 1) {  // 21, 31, 41, 51, ..., 81 -> true
    return true;
  }
  if (row == column) {  // square 22,33,44,55,66,77,88
    return true;
  }
  return false;
}

int ThumbnailProcesser::CreateThumbnailImages(const QStringList& files, int dimensionX, int dimensionY, int widthPx, const int timePeriod, const bool isJpg) {
  if (!IsDimensionXValid(dimensionX)) {
    qWarning("Dimension of row[%d] out of range", dimensionX);
    return -1;
  }
  if (!IsDimensionYValid(dimensionY)) {
    qWarning("Dimension of column[%d] out of range", dimensionY);
    return -1;
  }
  if (!IsWidthPixelAllowed(widthPx)) {
    qWarning("images width %d pixel(s) invalid", widthPx);
    return -1;
  }
  if (!IsSamplePeriodAllowed(timePeriod)) {
    qWarning("timePeriod: %d second invalid", timePeriod);
    return -1;
  }

  QMap<QString, QString> vidPath2ImgBaseName;
  for (const QString& pth : files) {
    const QFileInfo fi{pth};
    if (!fi.isFile()) {
      continue;
    }
    if (fi.size() < 10 * 1024) {  // skip file size under 10k byte(s)
      continue;
    }
    const QString& ext = fi.suffix();
    if (!TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + ext)) {
      continue;
    }
    vidPath2ImgBaseName[pth] = pth.left(pth.size() - ext.size() - 1);
  }
  if (vidPath2ImgBaseName.isEmpty()) {
    qDebug("no videos find need to create thumbnail image(s)");
    return 0;
  }

  int succeedCnt{0};
  const QString ffmpegExePath = "ffmpeg";
  // ffmpeg.exe -i "aa.mp4" -vf "select=key,scale=1080:-1,tile=2x2:margin=0:padding=0" -frames:v 1 -q:v 2 "aa 22.jpg"
  // for key frame: select=key
  // for time axis: fps=1/T
  QString config;
  config += QString{R"( -y -vf "fps=1/%1,)"}.arg(timePeriod);
  config += QString{R"(scale=%1:-1,tile=%2x%3:margin=0:padding=0" -frames:v 1 )"}  //
                .arg(widthPx)
                .arg(dimensionX)
                .arg(dimensionY);
  // for jpg add this one quality value 2: -q:v 2
  if (isJpg) {
    config += " -q:v 2";
  }
  const QString imgthumbnailImgExt{isJpg ? ".jpg" : ".png"};
  // imgThumbnailSuffix e.g., " 33.png", " 24.jpg"
  const QString imgThumbnailSuffix{" " + QString::number(dimensionX) + QString::number(dimensionY) + imgthumbnailImgExt};
  for (auto it = vidPath2ImgBaseName.cbegin(); it != vidPath2ImgBaseName.cend(); ++it) {
    const QString ffmpegCmdTemplate = ffmpegExePath     // executable path
                                      + R"( -i "%1" )"  // input path
                                      + config          //
                                      + R"( "%2")";     // output path
    const QString vidPath = PathTool::sysPath(it.key());
    const QString& path2imgBaseName = PathTool::sysPath(it.value()) + imgThumbnailSuffix;
    const QString ffmpegCmd{ffmpegCmdTemplate.arg(vidPath, path2imgBaseName)};
    QByteArray cmdBytes = ffmpegCmd.toLocal8Bit();
    int result = system(cmdBytes.constData());
    if (result == 0) {
      ++succeedCnt;
    } else {
      qWarning("Create thumbnail image for video[%s] failed with command: %s", qPrintable(vidPath), qPrintable(ffmpegCmd));
      return succeedCnt;
    }
  }
  return succeedCnt;
}

std::pair<int, int> ThumbnailProcesser::GetThumbnailDimension(const QString& imgBaseName) {
  if (imgBaseName.size() < 2) {
    return {-1, -1};
  }
  int row = imgBaseName[imgBaseName.size() - 2].digitValue();
  int column = imgBaseName[imgBaseName.size() - 1].digitValue();
  return {row, column};
}

int ThumbnailProcesser::operator()(const QString& rootPath, int beg, int end) {
  mErrImg.clear();
  mExtractImagesCnt = 0;
  mRewriteImagesCnt = 0;
  if (beg < 0 || beg >= end) {
    qWarning("image index range [%d, %d) is invalid", beg, end);
    return -1;
  }
  if (!QFileInfo(rootPath).isDir()) {
    qWarning("image rootpath[%s] is not a directory", qPrintable(rootPath));
    return -1;
  }
  QDirIterator it{rootPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    const QString imgAbsPath = it.next();
    if (!IsImageAnThumbnail(imgAbsPath)) {
      continue;
    }

    QString thumbnailImgBaseName;
    QString ext;
    // e.g., imgAbsPath="an image 44.jpeg"
    // thumbnailImgBaseName = "an image 44", ext=".jpeg", ImgBaseName = "an image"
    std::tie(thumbnailImgBaseName, ext) = PathTool::GetBaseNameExt(imgAbsPath);
    const QString ImgBaseName = thumbnailImgBaseName.chopped(3);  // "an image"

    // get thumbnail dimension by file name last 3 charactor
    int row{-1}, column{-1};
    std::tie(row, column) = GetThumbnailDimension(thumbnailImgBaseName);
    const int imgsCntIntThumbnail = row * column;
    const int endIndex = std::min(end, imgsCntIntThumbnail);
    if (beg >= endIndex) {
      continue;
    }
    QPixmap pixmap{imgAbsPath};
    const int wPixels = pixmap.width(), hPixels = pixmap.height();
    if (wPixels % row != 0 || hPixels % column != 0) {
      qDebug("thumbnail[%s] %d-by-%d pixels cannot be seperated into %d x %d screenshot",  //
             qPrintable(imgAbsPath), wPixels, hPixels, row, column);
      continue;
    }
    const int eachImgWidth{wPixels / row}, eachImgHeight{hPixels / column};
    if (!IsWidthPixelAllowed(eachImgWidth)) {
      continue;
    }

    QString imgAbsolutePthTemplate = PathTool::absolutePath(imgAbsPath);
    imgAbsolutePthTemplate += '/';
    imgAbsolutePthTemplate += ImgBaseName;
    imgAbsolutePthTemplate += ' ';
    imgAbsolutePthTemplate += "%1";
    imgAbsolutePthTemplate += ext;

    for (int imgInd = beg; imgInd < endIndex; ++imgInd) {
      int iRow = imgInd / column;
      int jColumn = imgInd % column;
      int x = iRow * eachImgWidth;
      int y = jColumn * eachImgHeight;

      const QString newImgAbsPth = imgAbsolutePthTemplate.arg(imgInd);
      if (QFile::exists(newImgAbsPth)) {
        if (mSkipImageIfAlreadyExist) {
          continue;
        }
        ++mRewriteImagesCnt;
        qWarning("Rewrite image[%s]", qPrintable(newImgAbsPth));
        mErrImg << QString("Rewrite image[%1]\n").arg(newImgAbsPth);
      }
      QPixmap newPixmap = pixmap.copy(x, y, eachImgWidth, eachImgHeight);
      if (!newPixmap.save(newImgAbsPth)) {
        qCritical("Save image failed[%s]", qPrintable(newImgAbsPth));
        mErrImg << QString("Save image failed[%1]\n").arg(newImgAbsPth);
        continue;
      }
      ++mExtractImagesCnt;
    }
  }
  return mExtractImagesCnt;
}
