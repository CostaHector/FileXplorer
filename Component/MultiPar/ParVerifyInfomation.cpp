#include "ParVerifyInfomation.h"
#include "Logger.h"
#include "PathTool.h"
#include <QRegularExpression>

QByteArray GetLastLineFromFormattedCliByteArray(QByteArray& cliOutput) {
  if (!cliOutput.isEmpty() && cliOutput.back() == '\n') {
    cliOutput.chop(1);
  }
  const int lastNewLineStart{cliOutput.lastIndexOf('\n')};
  if (lastNewLineStart == -1) {
    return "";
  }
  return cliOutput.mid(lastNewLineStart + 1);
}

// "PAR File list :"
// "Input File list      :"
QStringList ParseFileNames(const char* keySpaceColon, const int keySpaceColonCharCnt, const QByteArray& cliOutput, int searchStartPos = 0) {
  const int parFileListIndex = cliOutput.indexOf(keySpaceColon, searchStartPos);
  if (parFileListIndex == -1) {
    LOG_W("key[%s] not found", keySpaceColon);
    return {};
  }
  int newLineIndex = parFileListIndex + keySpaceColonCharCnt;
  newLineIndex = cliOutput.indexOf('\n', newLineIndex);
  if (newLineIndex == -1) {
    return {};
  }
  newLineIndex = cliOutput.indexOf('\n', newLineIndex + 1);
  if (newLineIndex == -1) {
    return {};
  }

  QStringList filesList;

  static constexpr const char COLON_2_NAME[]{R"( : ")"};
  static constexpr const int COLON_2_NAME_CHAR_CNT{sizeof(COLON_2_NAME) - 1};
  while (newLineIndex + 1 < cliOutput.size() && cliOutput[newLineIndex + 1] != '\n') {
    const int colon2NameIndex = cliOutput.indexOf(COLON_2_NAME, newLineIndex);
    if (colon2NameIndex == -1) {
      LOG_W("colon[%s] not found", COLON_2_NAME);
      return {};
    }

    const int parNameStart{colon2NameIndex + COLON_2_NAME_CHAR_CNT};
    newLineIndex = cliOutput.indexOf('\n', newLineIndex + 1);
    if (newLineIndex == -1) {
      return {};
    }

    const int nameLength = newLineIndex - parNameStart - 1;
    QString fileName = cliOutput.mid(parNameStart, nameLength);
    filesList.push_back(std::move(fileName));
  }
  return filesList;
}

bool ParVerifyInfomation::ParsePar2FilesList() noexcept {
  int searchStartPos{0};
  {
    static constexpr const char BASE_DIRECTORY_FIELD[]{"Base Directory\t: "};
    static constexpr const int BASE_DIRECTORY_FIELD_CHAR_CNT{sizeof(BASE_DIRECTORY_FIELD) - 1};
    const int baseDirectoryIndex = m_verifyOutput.indexOf(BASE_DIRECTORY_FIELD, searchStartPos);
    if (baseDirectoryIndex == -1) {
      LOG_W("key[%s] not found", BASE_DIRECTORY_FIELD);
      return false;
    }
    const int startOfPrePath = baseDirectoryIndex + BASE_DIRECTORY_FIELD_CHAR_CNT + 1;
    const int endOfPrePath = m_verifyOutput.indexOf('\n', startOfPrePath);
    searchStartPos = endOfPrePath;
    if (endOfPrePath == -1) {
      return false;
    }
    m_prePath = PathTool::normPath(m_verifyOutput.mid(startOfPrePath, endOfPrePath - startOfPrePath - 1));
  }
  {
    static constexpr const char PAR_FILE_LIST_FIELD[]{"PAR File list :"};
    static constexpr const int PAR_FILE_LIST_FIELD_CHAR_CNT{sizeof(PAR_FILE_LIST_FIELD) - 1};
    QStringList parFiles = ParseFileNames(PAR_FILE_LIST_FIELD, PAR_FILE_LIST_FIELD_CHAR_CNT, m_verifyOutput, searchStartPos);
    if (parFiles.isEmpty()) {
      LOG_W("Cannot find parFiles");
      return false;
    }
    m_par2FileName = parFiles.front();
    m_volPar2FileNameList = parFiles.mid(1);
  }
  {
    static constexpr const char SRC_FILE_LIST_FIELD[]{"Input File list      :"};
    static constexpr const int SRC_FILE_LIST_FIELD_CHAR_CNT{sizeof(SRC_FILE_LIST_FIELD) - 1};
    QStringList inputSrcFiles = ParseFileNames(SRC_FILE_LIST_FIELD, SRC_FILE_LIST_FIELD_CHAR_CNT, m_verifyOutput, searchStartPos);
    if (inputSrcFiles.isEmpty()) {
      LOG_W("Cannot find inputSrcFiles");
      return false;
    }
    m_srcFileNames.swap(inputSrcFiles);
  }
  return true;
}

void ParVerifyInfomation::ParseCountInfo() noexcept {
  static const QRegularExpression completeRegex("Complete file count\\s*:\\s*(\\d+)");
  static const QRegularExpression misnamedRegex("Misnamed file count\\s*:\\s*(\\d+)");
  static const QRegularExpression damagedRegex("Damaged file count\\s*:\\s*(\\d+)");
  static const QRegularExpression missingRegex("Missing file count\\s*:\\s*(\\d+)");

  static const QRegularExpression repairRegex("Repaired file count\\s*:\\s*(\\d+)");
  static const QRegularExpression restoreRegex("Restored file count\\s*:\\s*(\\d+)");
  QRegularExpressionMatch completeMatch = completeRegex.match(m_verifyOutput);
  if (completeMatch.hasMatch()) {
    m_completeCount = completeMatch.captured(1).toInt();
  }

  QRegularExpressionMatch misnamedMatch = misnamedRegex.match(m_verifyOutput);
  if (misnamedMatch.hasMatch()) {
    m_misnamedCount = misnamedMatch.captured(1).toInt();
    int restoreCnt{0};
    QRegularExpressionMatch restoreMatch = restoreRegex.match(m_verifyOutput);
    if (restoreMatch.hasMatch()) {
      restoreCnt = restoreMatch.captured(1).toInt();
    }
    if (restoreCnt > 0) {
      m_completeCount += restoreCnt;
      if (m_misnamedCount > 0) { m_misnamedCount -= restoreCnt; }
    }
  }

  QRegularExpressionMatch damagedMatch = damagedRegex.match(m_verifyOutput);
  if (damagedMatch.hasMatch()) {
    m_damagedCount = damagedMatch.captured(1).toInt();
    int repairedCnt{0};
    QRegularExpressionMatch repairMatch = repairRegex.match(m_verifyOutput);
    if (repairMatch.hasMatch()) {
      repairedCnt = repairMatch.captured(1).toInt();
    }
    if (repairedCnt > 0) {
      m_completeCount += repairedCnt;
      if (m_damagedCount > 0) { m_damagedCount -= repairedCnt; }
    }
  }

  QRegularExpressionMatch missingMatch = missingRegex.match(m_verifyOutput);
  if (missingMatch.hasMatch()) {
    m_missingCount = missingMatch.captured(1).toInt();
  }
}

bool ParVerifyInfomation::ParseStatusStr() noexcept {
  m_statusByteArray = GetLastLineFromFormattedCliByteArray(m_verifyOutput);
  if (m_statusByteArray.isEmpty()) {
    LOG_W("last line is empty.");
    return false;
  }
  if (m_statusByteArray.contains("All Files Complete")) {
    m_statusE = Par2StatusE::ALL_FILES_COMPLETE;
  } else if (m_statusByteArray.contains("Ready to repair ")) {
    m_statusE = Par2StatusE::READY_TO_REPAIR;
  } else if (m_statusByteArray.contains("Ready to rename ")) {
    m_statusE = Par2StatusE::READY_TO_RENAME;
  } else if (m_statusByteArray.contains("Need") && m_statusByteArray.contains("more slice") && m_statusByteArray.contains("repair")) {
    m_statusE = Par2StatusE::NEED_MORE_TO_REPAIR;
  } else if (m_statusByteArray ==	"Repaired successfully") {
    m_statusE = Par2StatusE::ALL_FILES_COMPLETE;
  } else {
    m_statusE = Par2StatusE::CANNOT_VERIFY;
    return false;
  }
  return true;
}

bool ParVerifyInfomation::init() noexcept {
  ParseCountInfo();
  if (!ParsePar2FilesList()) {
    return false;
  }
  if (!ParseStatusStr()) {
    return false;
  }
  return true;
}

bool ParVerifyInfomation::updateCliOutput(QByteArray& newCliOutput) {
  m_verifyOutput = newCliOutput;
  return init();
}

ParVerifyInfomation::ParVerifyInfomation(QByteArray&& cliOutput) noexcept
  : m_verifyOutput{std::move(cliOutput)} {
  init();
}
