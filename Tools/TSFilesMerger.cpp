#include "TSFilesMerger.h"
#include "Logger.h"
#include "FileTool.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>

// ffmpeg.exe and it's dependency required
namespace TSFilesMerger {
bool checkTsFilesConsistent(const QStringList& tsAbsPathList) {
  QProcess ffprobe;
  ffprobe.start("ffprobe", QStringList() << "-v" << "error" << "-show_streams" << tsAbsPathList.first());
  ffprobe.waitForFinished();

  QString referenceOutput = ffprobe.readAllStandardOutput();
  for (int i = 1; i < tsAbsPathList.size(); ++i) {
    const QString path = tsAbsPathList[i];
    ffprobe.start("ffprobe", QStringList() << "-v" << "error" << "-show_streams" << path);
    ffprobe.waitForFinished();
    QString tempOutput = ffprobe.readAllStandardOutput();
    if (tempOutput != referenceOutput) {
      return false;
    }
  }
  return true;
}

std::pair<bool, QString> mergeTsFiles(const QString& filesIn, const QStringList& tsNames) {
  if (tsNames.isEmpty()) {
    LOG_D("no input ts file");
    return {true, ""};
  }

  const QDir dir{filesIn};
  if (!dir.exists()) {
    LOG_D("Input path[%s] not exist", qPrintable(filesIn));
    return {false, ""};
  }

  static constexpr QChar SINGLE_QUOTE{'\''};
  // Create temp lists file (Each line pattern "file 'ts Absolute File Path'")
  QString fileListContent;
  int tsFilesCnt{0};
  for (const QString& name : tsNames) {
    if (!name.endsWith(".ts", Qt::CaseSensitivity::CaseInsensitive)) {
      continue;
    }
    if (!dir.exists(name)) {
      continue;
    }
    QString tsAbsPath = dir.absoluteFilePath(name);
    if (tsAbsPath.contains(SINGLE_QUOTE)) {
      LOG_W("File path[%s] contain invalid char", qPrintable(tsAbsPath));
      return {false, ""};
    }
    fileListContent += QString("file '%1'").arg(tsAbsPath);
    fileListContent += '\n';
    ++tsFilesCnt;
  }
  if (tsFilesCnt < 2) {
    LOG_D("Only %d ts file in input, no need merge! skip now.", tsFilesCnt);
    return {false, ""};
  }

  const QString parentFolderName{QFileInfo(filesIn).baseName()};

  const QString outputMergedTsFileAbsPath{dir.absoluteFilePath(parentFolderName + ".ts")};
  if (QFile::exists(outputMergedTsFileAbsPath)) {
    LOG_W("Output file already exist, skip merge into[%s]", qPrintable(outputMergedTsFileAbsPath));
    return {false, ""};
  }
  if (outputMergedTsFileAbsPath.contains(SINGLE_QUOTE)) {
    LOG_W("Output merged ts file path[%s] contain invalid char", qPrintable(outputMergedTsFileAbsPath));
    return {false, ""};
  }

  const QString inputFileAbsPath{dir.absoluteFilePath(parentFolderName + ".txt")};
  if (!FileTool::StringTextWriter(inputFileAbsPath, fileListContent, QIODevice::WriteOnly | QIODevice::Truncate)) {
    LOG_D("Open temp list file[%s] to write ts files failed", qPrintable(inputFileAbsPath));
    return {false, ""};
  }

  const QStringList args{"-f", "concat", "-safe", "0", "-i", inputFileAbsPath, "-c", "copy", outputMergedTsFileAbsPath};
  QProcess ffmpeg; // call FFmpeg to merge files
  ffmpeg.start("ffmpeg", args);
  const bool mergeRet = ffmpeg.waitForFinished(); // block and wait for finish
  QString cliOutput = ffmpeg.readAllStandardOutput();
  cliOutput += ffmpeg.readAllStandardError();
  if (!mergeRet || cliOutput.contains("Error", Qt::CaseSensitivity::CaseInsensitive)) {
    LOG_W("Merge %d ts file(s) into a large ts file[%s] failed. output:[%s]",
          tsFilesCnt, qPrintable(outputMergedTsFileAbsPath), qPrintable(cliOutput));
    return {false, ""};
  }

  if (!QFile::remove(inputFileAbsPath)) {
    LOG_W("Remove temp list file[%s] failed", qPrintable(inputFileAbsPath));
    return {true, outputMergedTsFileAbsPath};
  }
  LOG_D("[Ok] Merge %d ts file(s) into a large ts file[%s]", tsFilesCnt, qPrintable(outputMergedTsFileAbsPath));
  return {true, outputMergedTsFileAbsPath};
}

}
