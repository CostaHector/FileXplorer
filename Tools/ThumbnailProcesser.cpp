#include "ThumbnailProcesser.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"
#include "VideoDurationGetter.h"
#include "UndoRedo.h"
#include "CastBrowserHelper.h"
#include <QDirIterator>
#include <QRegularExpression>
#include <utility>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QSet>
#include <QProcess>
#include <QThread>

const QList<int> ThumbnailProcesser::mAllowedPixelList{360, 480, 720, 1080};
constexpr int ThumbnailProcesser::SAMPLE_PERIOD_MIN; // [1, 300)

bool ThumbnailProcesser::IsDimensionXValid(int dimensionX) {
  static constexpr int DIMENSION_X_MIN{1}, DIMENSION_X_MAX{10 + 1}; // [1, 10+1)
  return DIMENSION_X_MIN <= dimensionX && dimensionX <= DIMENSION_X_MAX;
}

bool ThumbnailProcesser::IsDimensionYValid(int dimensionY) {
  static constexpr int DIMENSION_Y_MIN{1}, DIMENSION_Y_MAX{10 + 1}; // [1, 10+1)
  return DIMENSION_Y_MIN <= dimensionY && dimensionY <= DIMENSION_Y_MAX;
}

bool ThumbnailProcesser::IsWidthPixelAllowed(int widthPixel) {
  static const QSet<int> mAllowedPixelSet{mAllowedPixelList.begin(), mAllowedPixelList.cend()};
  return mAllowedPixelSet.contains(widthPixel);
}

bool ThumbnailProcesser::IsSamplePeriodAllowed(int samplePeriod) {
  return samplePeriod >= SAMPLE_PERIOD_MIN;
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
  return true;
}

ThumbnailProcesser::ThumbnailProcesser(bool skipIfImgAlreadyExist) //
  : mSkipImageIfAlreadyExist{skipIfImgAlreadyExist}                //
{}

bool ThumbnailProcesser::IsImageAnThumbnail(const QString& imgAbsPath) {
  QString imgBaseName;
  QString ext;
  std::tie(imgBaseName, ext) = PathTool::GetBaseNameExt(imgAbsPath);
  if (!TYPE_FILTER::isDotExtImage(ext)) { // not an image
    return false;
  }
  if (!IsImageNameLooksLikeThumbnail(imgBaseName)) { // name not like
    return false;
  }
  QFileInfo fi{imgAbsPath};
  if (!fi.isFile()) {
    return false;
  }
  static constexpr int IMG_LARGEST_SIZE = 5 * 1024 * 1024; // 5MiB
  if (fi.size() > IMG_LARGEST_SIZE) {                      // images too large
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
  if (!IsDimensionXValid(row) || !IsDimensionXValid(column)) { // 0x, x0, row>=9 or column>=9
    return false;
  }
  if (row == 1) { // 1x -> false
    return false;
  }
  if (column == 1) { // 21, 31, 41, 51, ..., 81 -> true
    return true;
  }
  if (row == column) { // square 22,33,44,55,66,77,88
    return true;
  }
  return false;
}

bool ThumbnailProcesser::RenameThumbnailGeneratedByPotPlayer(const QString& path) const {
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds;

  QDirIterator it{path, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  const static QSet<QString> VID_TYPE_HASH{TYPE_FILTER::VIDEO_TYPE_SET.cbegin(), TYPE_FILTER::VIDEO_TYPE_SET.cend()};

  QString directFolderPath;
  QString oldImgPath, oldImgName;
  QString correspondFile, imgDotExt;
  QString vidBaseName, vidDotext;
  while (it.hasNext()) {
    oldImgPath = it.next();
    std::tie(correspondFile, imgDotExt) = PathTool::GetBaseNameExt(oldImgPath);
    std::tie(vidBaseName, vidDotext) = PathTool::GetBaseNameExt(correspondFile);
    directFolderPath = oldImgPath.chopped(1 + correspondFile.size() + imgDotExt.size());
    oldImgName = oldImgPath.mid(directFolderPath.size() + 1);
    if (!VID_TYPE_HASH.contains("*" + vidDotext)) {
      continue;
    }

    const int imgWidth = CastBrowserHelper::GetImageSize(oldImgPath).width();
    if (imgWidth % 360 != 0 && imgWidth % 480 != 0) { // width invalid
      continue;
    }
    const int gridCnt{ imgWidth % 720 == 0 ? imgWidth / 720 : (imgWidth % 480 == 0 ? imgWidth / 480 : imgWidth / 360) };

    QDir dir{directFolderPath};
    if (!dir.exists(correspondFile)) {
      continue;
    }
    // newImageName = vidBaseName + " 33" + imgDotExt
    vidBaseName += QString::asprintf(" %d%d", gridCnt, gridCnt);
    vidBaseName += imgDotExt;
    if (dir.exists(vidBaseName)) {
      if (mSkipImageIfAlreadyExist) {
        LOG_D("newImageName occupied[%s] skip it", qPrintable(vidBaseName));
        continue;
      }
      cmds.push_back(ACMD::GetInstMOVETOTRASH(directFolderPath, vidBaseName));
    }
    cmds.push_back(ACMD::GetInstRENAME(directFolderPath, oldImgName, vidBaseName));
  }
  if (cmds.isEmpty()) {
    return true;
  }
  auto& undoRedo = UndoRedo::GetInst();
  return undoRedo.Do(cmds);
}

int ThumbnailProcesser::CreateThumbnailImages(const QStringList& files, int dimensionX, int dimensionY, int widthPx, const bool isJpg) const {
  if (files.isEmpty()) {
    return 0;
  }
  if (!CheckParameters(dimensionX, dimensionY, widthPx)) {
    return -1;
  }
  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    return -1;
  }

  const QString imgThumbnailSuffix{QString::asprintf(" %d%d.%s", dimensionX, dimensionY, (isJpg ? "jpg" : "png"))};
  const QString ffmpegExePath{"ffmpeg"};

  int succeedCnt{0};
  const int threadCount = QThread::idealThreadCount(); // 获取CPU核心数

  for (const QString& pth : files) {
    const QFileInfo fi{pth};
    if (!fi.isFile()) {
      continue;
    }
    const QString& ext = fi.suffix();
    if (!TYPE_FILTER::isDotExtVideo("." + ext)) {
      continue;
    }
#ifndef RUNNING_UNIT_TESTS
    if (fi.size() < 50 * 1024 * 1024) { // skip: if file size under 50 MiB
      LOG_D("Skip file[%s] size under threshold", qPrintable(pth));
      continue;
    }
#endif
    const QString vidPath = PathTool::sysPath(pth);
    const QString vidBaseName = pth.left(pth.size() - ext.size() - 1);
    const QString path2imgBaseName = PathTool::sysPath(vidBaseName) + imgThumbnailSuffix;
    if (mSkipImageIfAlreadyExist && QFile::exists(path2imgBaseName)) { // skip if name xx.jpg already exist
      LOG_D("Skip file[%s] thumbnail already exist", qPrintable(pth));
      continue;
    }

    const int dur = mi.GetLengthQuick(pth);                // unit: ms
    int timePeriod = dur / dimensionX / dimensionY / 1000; // unit: second
#ifdef RUNNING_UNIT_TESTS
    const double startTime = 0;
#else
    if (!IsSamplePeriodAllowed(timePeriod)) { // skip: if duration under 1 second
      LOG_D("Skip file[%s] duration under threshold", qPrintable(pth));
      continue;
    }
    if (dimensionX == dimensionY && dimensionX == 1 && timePeriod > 5) {
      timePeriod = 5; // speed up
    }
    const double startTime = std::max(5.0, dur / 1000.0 * 0.01); // skip first 5 seconds, or 1%
#endif

    // 构建FFmpeg命令参数列表
    QStringList ffmpegArgs;
    ffmpegArgs.reserve(20);
    ffmpegArgs << "-y" << "-loglevel" << "error";
    ffmpegArgs << "-threads" << QString::number(threadCount);
    // ffmpegArgs << "-hwaccel" << "auto" << "-noaccurate_seek"; // best coverage
    ffmpegArgs << "-ss" << QString::number(startTime);        // 智能定位
    ffmpegArgs << "-i" << vidPath;                            // 输入文件

    // 构建滤镜参数
    QString filter{QString::asprintf(
        // "select='key',"
        "fps=1/%d,"
        "scale=%d:-1,"
        "tile=%dx%d:margin=0:padding=0",
        timePeriod, widthPx, dimensionY, dimensionX)};
    ffmpegArgs << "-vf" << filter;
    ffmpegArgs << "-frames:v" << "1";
    if (isJpg) {
      ffmpegArgs << "-q:v" << "3";
    }

    ffmpegArgs << path2imgBaseName; // 输出文件
    // 使用推荐的start()重载
    QProcess ffmpegProcess;
    ffmpegProcess.start(ffmpegExePath, ffmpegArgs);

    // 智能超时设置
    const int timeoutMs = std::min(dur + 30000, 300000);
    if (!ffmpegProcess.waitForFinished(timeoutMs)) {
      LOG_W("FFmpeg timeout for video[%s] after %d ms", qPrintable(vidPath), timeoutMs);
      ffmpegProcess.kill();
      continue;
    }
    if (ffmpegProcess.exitCode() != 0) {
      const QString errorOutput = QString::fromLocal8Bit(ffmpegProcess.readAllStandardError());
      LOG_W("Create thumbnail image for video[%s] failed with command: %s\nError: %s",
            qPrintable(vidPath),
            qPrintable(ffmpegExePath + " " + ffmpegArgs.join(" ")),
            qPrintable(errorOutput));
      continue;
    }
    ++succeedCnt;
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
    LOG_W("image index range [%d, %d) is invalid", beg, end);
    return -1;
  }
  if (!QFileInfo(rootPath).isDir()) {
    LOG_W("image rootpath[%s] is not a directory", qPrintable(rootPath));
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
    const QString ImgBaseName = thumbnailImgBaseName.chopped(3); // "an image"

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
      LOG_D("thumbnail[%s] %d-by-%d pixels cannot be seperated into %d x %d screenshot", //
            qPrintable(imgAbsPath),
            wPixels,
            hPixels,
            row,
            column);
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
        LOG_W("Rewrite image[%s]", qPrintable(newImgAbsPth));
        mErrImg << QString("Rewrite image[%1]\n").arg(newImgAbsPth);
      }
      QPixmap newPixmap = pixmap.copy(x, y, eachImgWidth, eachImgHeight);
      if (!newPixmap.save(newImgAbsPth)) {
        LOG_C("Save image failed[%s]", qPrintable(newImgAbsPth));
        mErrImg << QString("Save image failed[%1]\n").arg(newImgAbsPth);
        continue;
      }
      ++mExtractImagesCnt;
    }
  }
  return mExtractImagesCnt;
}
