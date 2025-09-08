#include "TSFilesMerger.h"
#include "Logger.h"
#include <QProcess>
#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>

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
    LOG_D("Empty ts files input");
    return {true, ""};
  }

  const QDir dir{filesIn};
  if (!dir.exists()) {
    LOG_D("Input path not exist");
    return {false, ""};
  }

  // Create temp lists file (Each line pattern "file 'ts Absolute File Path'")
  QString fileListContent;
  QString tsAbsPath;
  int tsFilesCnt{0};
  for (const QString& name : tsNames) {
    if (!dir.exists(name)) {
      continue;
    }
    tsAbsPath = dir.absoluteFilePath(name);
    if (!name.endsWith(".ts", Qt::CaseSensitivity::CaseInsensitive)) {
      continue;
    }
    fileListContent += QString("file '%1'").arg(tsAbsPath);
    fileListContent += '\n';
    ++tsFilesCnt;
  }
  if (tsFilesCnt < 2) {
    LOG_D("Only %d ts file input, skip", tsFilesCnt);
    return {false, ""};
  }

  const QString parentFolderName{QFileInfo(filesIn).baseName()};
  const QString outputLargeTsFileName{parentFolderName + ".ts"};
  if (dir.exists(outputLargeTsFileName)) {
    LOG_W("Output file already exist, skip merge into[%s]", qPrintable(outputLargeTsFileName));
    return {false, ""};
  }

  const QString tempListFileName{parentFolderName + ".txt"};
  QFile tempListFile(dir.absoluteFilePath(tempListFileName));
  if (!tempListFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    LOG_D("Open temp list file to write ts files failed", tempListFile.fileName());
    return {false, ""};
  }
  QTextStream stream(&tempListFile);
  stream << fileListContent;
  tempListFile.close();

  const QString outputTsAbsPath{dir.absoluteFilePath(outputLargeTsFileName)};
  QProcess ffmpeg; // call FFmpeg to merge files
  QStringList args;
  args << "-f" << "concat" << "-safe" << "0" << "-i" << tempListFile.fileName()
       << "-c" << "copy" << outputTsAbsPath;
  ffmpeg.start("ffmpeg", args);
  const bool mergeRet = ffmpeg.waitForFinished(); // block and wait for finish
  if (!mergeRet) {
    LOG_D("[Failed] Merge %d ts file(s) into a large ts file[%s]", tsNames.size(), qPrintable(outputTsAbsPath));
    return {false, ""};
  }

  if (!tempListFile.remove()) {
    LOG_W("Remove temp list file[%s] failed", qPrintable(tempListFile.fileName()));
    return {true, outputTsAbsPath};
  }
  LOG_D("[Ok] Merge %d ts file(s) into a large ts file[%s]", tsNames.size(), qPrintable(outputTsAbsPath));
  return {true, outputTsAbsPath};
}

}
