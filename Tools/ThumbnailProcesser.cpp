#include "ThumbnailProcesser.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include "Tools/VideoDurationGetter.h"
#include <QRegularExpression>
#include <utility>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>

using namespace PathTool;

ThumbnailProcesser::ThumbnailProcesser(bool skipIfImgAlreadyExist) : mSkipImageIfAlreadyExist{skipIfImgAlreadyExist} {}

bool ThumbnailProcesser::IsImageAnThumbnail(const QString& imgAbsPath) {
  QString imgBaseName;
  QString ext;
  std::tie(imgBaseName, ext) = GetBaseNameExt(imgAbsPath);
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
  if (row == 1) {  // 11, 12, ..., 19 -> false
    return false;
  }
  if (column == 1) {  // 21, 31, 41, 51, ..., 91 -> true
    return true;
  }
  if (row == column && (2 <= row && row <= 5)) {  // square 22,33,44,55
    return true;
  }
  return false;
}

int ThumbnailProcesser::CreateThumbnailImages(int dimensionX, int dimensionY, int widthPx, const QStringList& files, const bool isJpg, const int timePeriod) {
  if (!(1 <= dimensionX && dimensionX <= 3)) {
    qWarning("Dimension of row[%d] invalid", dimensionX);
    return -1;
  }
  if (!(1 <= dimensionY && dimensionY <= 3)) {
    qWarning("Dimension of column[%d] invalid", dimensionY);
    return -1;
  }
  if (!(widthPx == 360 || widthPx == 480 || widthPx == 720 || widthPx == 1080)) {
    qWarning("images width %d pixel(s) invalid", widthPx);
    return -1;
  }
  if (timePeriod <= 0) {
    qWarning("timePeriod: %d second invalid", widthPx);
    return -1;
  }

  QMap<QString, QString> vidPath2ImgBaseName;
  for (const QString& pth : files) {
    const QFileInfo fi{pth};
    if (!fi.isFile()) {
      continue;
    }
    if (fi.size() < 10) {  // skip file size under 10 byte(s)
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
#ifdef _WIN32
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
  const QString dimensionStr = QString::number(dimensionX) + QString::number(dimensionY);
  const QString imgthumbnailImgExt{isJpg ? ".jpg" : ".png"};
  // e.g., " 33.png"
  const QString imgThumbnailSuffix{" " + dimensionStr + imgthumbnailImgExt};
  for (auto it = vidPath2ImgBaseName.cbegin(); it != vidPath2ImgBaseName.cend(); ++it) {
    const QString ffmpegCmdTemplate = ffmpegExePath     // executable path
                                      + R"( -i "%1" )"  // input path
                                      + config          //
                                      + R"( "%2")";     // output path

    const QString vidPath = PathTool::sysPath(it.key());
    const QString& path2imgBaseName = PathTool::sysPath(it.value()) + imgThumbnailSuffix;
    const QString ffmpegCmd{ffmpegCmdTemplate.arg(vidPath, path2imgBaseName)};
    qWarning("cmd:%s", qPrintable(ffmpegCmd));

    QByteArray cmdBytes = ffmpegCmd.toLocal8Bit();
    int result = system(cmdBytes.constData());
    if (result == 0) {
      ++succeedCnt;
    } else {
      qWarning("Create thumbnail image for video[%s] failed with command: %s", qPrintable(vidPath), qPrintable(ffmpegCmd));
      return succeedCnt;
    }
  }
#else
  qWarning("Create thumbnail image(s) only support in windoes platform now");
#endif
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
    int row{-1}, column{-1};

    QString thumbnailImgBaseName;
    QString ext;
    std::tie(thumbnailImgBaseName, ext) = GetBaseNameExt(imgAbsPath);
    const QString ImgBaseName = thumbnailImgBaseName.chopped(3);  // " 33"

    std::tie(row, column) = GetThumbnailDimension(thumbnailImgBaseName);
    const int imgsCntIntThumbnail = row * column;
    const int endIndex = std::min(end, imgsCntIntThumbnail);
    if (beg >= endIndex) {
      continue;
    }
    QPixmap pixmap(imgAbsPath);
    int wPixels = pixmap.width();
    int hPixels = pixmap.height();
    if (wPixels % row != 0 || hPixels % column != 0) {
      qDebug(
          "thumbnail[%s] %d-by-%d pixels cannot be seperated into "
          "%d x %d screenshot",
          qPrintable(imgAbsPath), wPixels, hPixels, row, column);
      continue;
    }
    int eachImgWidth{wPixels / row}, eachImgHeight{hPixels / column};
    if (!(eachImgWidth == 360 || eachImgWidth == 480 || eachImgWidth == 720 || eachImgWidth == 1080)) {
      continue;
    }

    for (int imgInd = beg; imgInd < endIndex; ++imgInd) {
      int iRow = imgInd / column;
      int jColumn = imgInd % column;
      int x = iRow * eachImgWidth;
      int y = jColumn * eachImgHeight;

      QString newImageAbsPath = absolutePath(imgAbsPath);
      newImageAbsPath += '/';
      newImageAbsPath += ImgBaseName;
      newImageAbsPath += ' ';
      newImageAbsPath += QString::number(imgInd);
      newImageAbsPath += ext;
      if (QFile::exists(newImageAbsPath)) {
        if (mSkipImageIfAlreadyExist) {
          continue;
        }
        ++mRewriteImagesCnt;
        qWarning("Rewrite image[%s]", qPrintable(newImageAbsPath));
        mErrImg << QString("Rewrite image[%1]\n").arg(newImageAbsPath);
      }
      QPixmap newPixmap = pixmap.copy(x, y, eachImgWidth, eachImgHeight);
      if (!newPixmap.save(newImageAbsPath)) {
        qCritical("Save image failed[%s]", qPrintable(newImageAbsPath));
        mErrImg << QString("Save image failed[%1]\n").arg(newImageAbsPath);
        continue;
      }
      ++mExtractImagesCnt;
    }
  }
  return mExtractImagesCnt;
}
