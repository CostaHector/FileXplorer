#include "MultiParTools.h"
#include "SystemPath.h"
#include "Logger.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>

namespace MultiParTools {
std::pair<bool, int> CreatePar2(const QStringList& filesAbsPath, int rateOfRedundancy) {
  if (filesAbsPath.isEmpty()) {
    return {true, 0};
  }
#ifndef _WIN32
  LOG_W("MultiPar not support in non-windows");
  return {false, 0};
#endif
  const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (multiParPath.isEmpty() || !QFile::exists(multiParPath)) {
    LOG_W("MultiPar[%s] not exist", qPrintable(multiParPath));
    return {false, 0};
  }

  QString rateOfredundancyParm{QString::asprintf("/rr%d", rateOfRedundancy)};

  int succeedCreatePar2Cnt{0};
  for (const QString& fileAbsPath : filesAbsPath) {
    if (!QFileInfo{fileAbsPath}.isFile()) { // not existed file
      continue;
    }

    const QString parFileAbsPath{fileAbsPath + ".par2"};
    if (QFile::exists(parFileAbsPath)) { // already exist
      continue;
    }

    QProcess crtProcess;
    const QStringList crtArgs{"c", rateOfredundancyParm, "/up", "/uo", parFileAbsPath, fileAbsPath};
    crtProcess.start(multiParPath, crtArgs);
    const bool crtPat2Ret = crtProcess.waitForFinished(); // block and wait for finish
    if (!crtPat2Ret) {
      LOG_D("[Failed] cannot create .par2 for file[%s]", qPrintable(fileAbsPath));
      return {false, succeedCreatePar2Cnt};
    }
    ++succeedCreatePar2Cnt;
  }
  LOG_D("Create .par2 for %d file(s) succeed", succeedCreatePar2Cnt);
  return {true, succeedCreatePar2Cnt};
}

// return: succeed, needRecovery
std::pair<bool, bool> isFileNeedRecovery(const QString& fileAbsPath, const QString& parFileAbsPath) {
  static const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (multiParPath.isEmpty() || !QFile::exists(multiParPath)) {
    LOG_W("MultiPar[%s] not exist", qPrintable(multiParPath));
    return {false, false};
  }
  if (!QFile::exists(fileAbsPath)) {
    return {true, true};
  }
  if (!QFile::exists(parFileAbsPath)) {
    return {true, false};
  }

  QProcess verifyProcess;
  const QStringList verifyArgs{"v", "/uo", parFileAbsPath, fileAbsPath};
  verifyProcess.start(multiParPath, verifyArgs);
  const bool verifyRet = verifyProcess.waitForFinished(); // block and wait for finish
  if (!verifyRet) {
    LOG_W("[Failed] cannot verify .par2 for file[%s]", qPrintable(fileAbsPath));
    return {false, true};
  }

  QString output = verifyProcess.readAllStandardOutput();
  output += verifyProcess.readAllStandardError();

  static const QRegularExpression completeRegex("Complete file count\\s*:\\s*(\\d+)");
  static const QRegularExpression misnamedRegex("Misnamed file count\\s*:\\s*(\\d+)");
  static const QRegularExpression damagedRegex("Damaged file count\\s*:\\s*(\\d+)");
  static const QRegularExpression missingRegex("Missing file count\\s*:\\s*(\\d+)");

  int completeCount = 1;
  int misnamedCount = 0;
  int damagedCount = 0;
  int missingCount = 0;
  QRegularExpressionMatch completeMatch = completeRegex.match(output);
  if (completeMatch.hasMatch()) {
    completeCount = completeMatch.captured(1).toInt();
  }
  QRegularExpressionMatch misnamedMatch = misnamedRegex.match(output);
  if (misnamedMatch.hasMatch()) {
    misnamedCount = misnamedMatch.captured(1).toInt();
  }
  QRegularExpressionMatch damagedMatch = damagedRegex.match(output);
  if (damagedMatch.hasMatch()) {
    damagedCount = damagedMatch.captured(1).toInt();
  }
  QRegularExpressionMatch missingMatch = missingRegex.match(output);
  if (missingMatch.hasMatch()) {
    missingCount = missingMatch.captured(1).toInt();
  }

  if (completeCount != 1 || missingCount > 0 || damagedCount > 0) {
    LOG_W("File broken[%s] complete:%d, misname:%d, damage:%d, missing:%d",
          qPrintable(fileAbsPath), //
          completeCount,
          misnamedCount,
          damagedCount,
          missingCount);
    return {true, true};
  }
  return {true, false};
}

std::pair<bool, int> VerifyPar2(const QStringList& filesAbsPath) {
  if (filesAbsPath.isEmpty()) {
    return {true, 0};
  }
#ifndef _WIN32
  LOG_W("MultiPar not support in non-windows");
  return {false, 0};
#endif

  static const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};

  QString recoverCommands;

  bool bVerifySucceed{false}, bNeedVerify{true};
  int needRecoverCnt{0};
  QSet<QString> mSrcFilesAlreadyMet;
  for (const QString& fileAbsPath : filesAbsPath) {
    QString par2File, srcFile;
    if (fileAbsPath.endsWith(".par2")) {
      srcFile = fileAbsPath.left(fileAbsPath.size() - 5);
      par2File = fileAbsPath;
      // par2File=fileAbsPath=a.mp4.vol0+1.par2
      // srcFile = a.mp4.vol0+1
      const int dotIndex{srcFile.lastIndexOf('.')};
      if (dotIndex > 0 && srcFile.mid(dotIndex + 1).startsWith("vol")) {
        mSrcFilesAlreadyMet.contains(srcFile.left(dotIndex));
        continue;
      }
    } else {
      par2File = fileAbsPath + ".par2";
      srcFile = fileAbsPath;
    }
    if (mSrcFilesAlreadyMet.contains(srcFile)) {
      continue;
    }
    mSrcFilesAlreadyMet.insert(srcFile);

    std::tie(bVerifySucceed, bNeedVerify) = isFileNeedRecovery(srcFile, par2File);
    if (!bVerifySucceed) {
      return {false, needRecoverCnt};
    }
    if (bNeedVerify) {
      QString recoverCmd{multiParPath};
      recoverCmd += " r /uo ";
      recoverCmd += '"';
      recoverCmd += par2File;
      recoverCmd += '"';
      recoverCmd += ' ';
      recoverCmd += '"';
      recoverCmd += srcFile;
      recoverCmd += '"';
      recoverCommands += recoverCmd;
      ++needRecoverCnt;
    }
  }
  if (needRecoverCnt > 0) {
    LOG_D("Below %d file(s) need recover:\n%s\n", needRecoverCnt, qPrintable(recoverCommands));
  }
  return {true, needRecoverCnt};
}

} // namespace MultiParTools
