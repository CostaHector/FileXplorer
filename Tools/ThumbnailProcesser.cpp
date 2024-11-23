#include "ThumbnailProcesser.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include <QRegularExpression>
#include <utility>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>

using namespace PATHTOOL;

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
