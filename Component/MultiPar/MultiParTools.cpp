#include "MultiParTools.h"
#include "SystemPath.h"
#include "Logger.h"
#include "Par2Tools.h"

#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSet>

namespace MultiParTools {
void FormatCliOuput(QByteArray& cliOutput) {
  cliOutput.replace("\r\n", "\n");
}

int clampRedundancy(int rateOfRedundancy) {
  // [10, 100]
  return std::max(std::min(100, rateOfRedundancy), 10);
}

bool IsMultiPar2Available(const QString& multiParPath) {
  if (multiParPath.isEmpty() || !QFile::exists(multiParPath)) {
    LOG_W("MultiPar[%s] not exist", qPrintable(multiParPath));
    return false;
  }
  return true;
}

std::pair<bool, int> CreatePar2(const QStringList& filesAbsPath, int rateOfRedundancy) {
  rateOfRedundancy = clampRedundancy(rateOfRedundancy);
  if (filesAbsPath.isEmpty()) {
    return {true, 0};
  }
#ifndef _WIN32
  LOG_W("MultiPar not support in non-windows");
  return {false, 0};
#endif
  const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (!IsMultiPar2Available(multiParPath)) {
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

ParVerifyInfomation VerifyAFile(const QString& multiParPath, const QString& parFileAbsPath) {
  QProcess verifyProcess;
  const QStringList verifyArgs{"v", "/uo", parFileAbsPath};
  verifyProcess.start(multiParPath, verifyArgs);
  const bool verifyRet = verifyProcess.waitForFinished(); // block and wait for finish
  if (!verifyRet) {
    LOG_W("[Failed] verify .par2 for file[%s]", qPrintable(parFileAbsPath));
    return ParVerifyInfomation{ParVerifyInfomation::Par2StatusE::CANNOT_VERIFY};
  }

  QByteArray cliOutput;
  cliOutput.reserve(500);
  cliOutput += verifyProcess.readAllStandardOutput();
  cliOutput += verifyProcess.readAllStandardError();
  FormatCliOuput(cliOutput);
  return ParVerifyInfomation{std::move(cliOutput)};
}

RepairResultE RepairAFile(const QString& multiParPath, const QString& parFileAbsPath, QByteArray* pRepairCliOutput) {
  QProcess repairProcess;
  const QStringList repairArgs{"r", "/uo", parFileAbsPath};
  repairProcess.start(multiParPath, repairArgs);
  const bool repairRet = repairProcess.waitForFinished(); // block and wait for finish
  if (!repairRet) {
    LOG_W("[Failed] verify .par2 for file[%s]", qPrintable(parFileAbsPath));
    return RepairResultE::CANNOT_REPAIR;
  }
  QByteArray cliOutput;
  cliOutput.reserve(500);
  cliOutput += repairProcess.readAllStandardOutput();
  cliOutput += repairProcess.readAllStandardError();
  FormatCliOuput(cliOutput);
  if (pRepairCliOutput != nullptr) {
    *pRepairCliOutput = cliOutput;
  }
  QByteArray lastLineContents = GetLastLineFromFormattedCliByteArray(cliOutput);
  // repaired or no need repait
  if (lastLineContents == "Repaired successfully") {
    return RepairResultE::SUCCESS_REPAIRED;
  }
  if (lastLineContents == "All Files Complete") {
    return RepairResultE::NO_NEED_REPAIR;
  }
  LOG_W("Repair[%s] failed[%s]", qPrintable(parFileAbsPath), lastLineContents.constData());
  return RepairResultE::NEED_MORE_TO_REPAIR;
}

// <errorMet, trustableList>
std::pair<bool, ParVerifyInfomationList> VerifyFiles(const QStringList& filesAbsPath) {
  if (filesAbsPath.isEmpty()) {
    return {true, ParVerifyInfomationList{}};
  }
  static const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (!IsMultiPar2Available(multiParPath)) {
    return {false, ParVerifyInfomationList{}};
  }

  QSet<QString> mSrcFilesAlreadyMet;
  QList<ParVerifyInfomation> ansList;
  for (const QString& par2fileAbsPath : filesAbsPath) {
    if (!par2fileAbsPath.endsWith(".par2")) {
      continue;
    }
    QString srcFileName = Par2Tools::ChopPostfixVolAndPar2(par2fileAbsPath);
    if (mSrcFilesAlreadyMet.contains(srcFileName)) {
      continue;
    }
    mSrcFilesAlreadyMet.insert(srcFileName);
    ansList.push_back(VerifyAFile(multiParPath, par2fileAbsPath));
  }
  LOG_D("Below %d file(s) checked ok", ansList.size());
  return {true, ansList};
}

} // namespace MultiParTools
