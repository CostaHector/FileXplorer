#include "DvdFileInfo.h"
#include "FileTool.h"
#include "VideoDurationGetter.h"
#include "MD5Calculator.h"
#include "PathTool.h"

#include <QTextStream>
#include <QFile>
#include <QDir>

namespace DvdFileInfo {
QString ReadDvdFileLine(const QString &dvdFilePath, DvdFileLineE line) {
  QFile file(dvdFilePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_W("Cannot open file[%s] for read", qPrintable(dvdFilePath));
    return {};
  }

  QTextStream in(&file);
  const int targetLine = static_cast<int>(line);
  for (int i = 0; i <= targetLine; ++i) {
    QString lineContent = in.readLine();
    if (i == targetLine) {
      if (lineContent.isEmpty()) {
        LOG_E("file[%s] target row[%d] is empty", qPrintable(dvdFilePath), targetLine);
        return {};
      }
      return lineContent;
    }
  }
  LOG_E("file[%s] not contain target row[%d]", qPrintable(dvdFilePath), targetLine);
  return {};
}

bool GenerateDvdFile(const QString &videoTsPath, bool bOverrideWhenExist, bool* bFailed) {
  QString parentAbsPath = PathTool::absolutePath(videoTsPath);
  QString dvdBaseName = PathTool::fileName(parentAbsPath);
  QString dvdAbsPath = PathTool::Path2Join(parentAbsPath, dvdBaseName + ".dvd");
  if (QFile::exists(dvdAbsPath) && QFile{dvdAbsPath}.size() > 0 && !bOverrideWhenExist) {
    return false;
  }

  QDir videoTsDir{videoTsPath, "*.vob", QDir::SortFlag::Name, QDir::Filter::Files};
  const int vobFilesCount = videoTsDir.count();
  if (vobFilesCount == 0) {
    return false;
  }
  qint64 totalSize{0};
  int totalDuration{0};

  // file size: qint64, each file < 4GB, 4.2*10^9, 10 digit is enough
  // video duration: 10 hour, 10*3600*1000ms, 3.6*10^7, 8 digit is enough
  static constexpr int EACH_SIZE_DIGIT = 10, EACH_DURATION_DIGIT = 8;
  QString sizeCsv;
  sizeCsv.reserve(vobFilesCount * (10 + 1));
  QString durationCsv;
  durationCsv.reserve(vobFilesCount * (8 + 1));
  QByteArray md5Csv;
  md5Csv.reserve(vobFilesCount * (MD5Calculator::FIXED_MD5_LENGTH + 1));

  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    if (bFailed != nullptr) *bFailed = true;
    return false;
  }

  for (const QString& vobFileName: videoTsDir.entryList()) {
    const QString vobFileAbsPath = videoTsDir.absoluteFilePath(vobFileName);

    qint64 size = QFile{vobFileAbsPath}.size();
    totalSize += size;
    sizeCsv += QString::number(size);
    sizeCsv += ',';

    int duration = VideoDurationGetter::GetLengthQuickStatic(mi, vobFileAbsPath);
    totalDuration += duration;
    durationCsv += QString::number(duration);
    durationCsv += ',';

    md5Csv += MD5Calculator::GetFileMD5(vobFileAbsPath, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
    md5Csv += ',';
  }
  QByteArray totalMd5 = MD5Calculator::GetByteArrayMD5(md5Csv);

  const int contentsLength {
      EACH_SIZE_DIGIT + 1 + EACH_DURATION_DIGIT + 1 + MD5Calculator::FIXED_MD5_LENGTH + 1 +
      sizeCsv.size()  + 1 + durationCsv.size()  + 1 + md5Csv.size() + 1
  };
  QString contents;
  contents.reserve(contentsLength);
  contents += QString::number(totalSize);
  contents += '\n';
  contents += QString::number(totalDuration);
  contents += '\n';
  contents += QString::fromLatin1(totalMd5);
  contents += '\n';
  contents += std::move(sizeCsv);
  contents += '\n';
  contents += std::move(durationCsv);
  contents += '\n';
  contents += std::move(md5Csv);
  contents += '\n';

  return FileTool::StringTextWriter(dvdAbsPath, contents, QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
}

}

