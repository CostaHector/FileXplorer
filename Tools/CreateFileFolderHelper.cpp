#include "CreateFileFolderHelper.h"
#include "FileOperatorPub.h"
#include "UndoRedo.h"
#include "NotificatorMacro.h"
#include "JsonHelper.h"
#include "JsonKey.h"
#include "PathTool.h"

#include <QDateTime>
#include <QDir>

namespace CreateFileFolderHelper {

bool NewPlainTextFile(const QString& createIn, QString* newTextFileAbsPath) {
  const QString plainTextFileName {QString("New Text Document %1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))};

  const QDir curDir{createIn};
  if (curDir.exists(plainTextFileName)) {
    LOG_OK_NP("Plain text file already exist", plainTextFileName);
    return true;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds{ACMD::GetInstTOUCH(curDir.absolutePath(), plainTextFileName)};
  if (!UndoRedo::GetInst().Do(cmds)) {
    LOG_ERR_NP("[Failed] Touch file", plainTextFileName);
    return false;
  }
  if (newTextFileAbsPath != nullptr) {
    *newTextFileAbsPath = curDir.absoluteFilePath(plainTextFileName);
  }
  return true;
}

int NewJsonFile(const QString& createIn, const QStringList& basedOnFileNames) {
  if (basedOnFileNames.isEmpty()) {
    LOG_INFO_NP("[Skip] New Json file, nothing selected", "return");
    return 0;
  }
  int crtSucceed{0};
  int totalNeedCnd{0};
  QString jsonBaseName, ext;
  for (const QString& fileItem : basedOnFileNames) {
    std::tie(jsonBaseName, ext) = PathTool::GetBaseNameExt(fileItem);
    const QString jPath = createIn + '/' + jsonBaseName + ".json";
    if (QFile::exists(jPath)) {
      continue;
    }
    ++totalNeedCnd;
    const auto& dict = JsonKey::GetJsonDictDefault(jsonBaseName);
    if (JsonHelper::DumpJsonDict(dict, jPath)) {
      ++crtSucceed;
    }
  }
  LOG_OK_P("[Ok]Json file(s) create", "count: succ:%d/total:%d/selected:%d", crtSucceed, totalNeedCnd, basedOnFileNames.size());
  return crtSucceed;
}

bool NewFolder(const QString& createIn, QString* folderAbsPath) {
  const QString& newFolderName = QString("New Folder %1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
  QDir createInDir{createIn};
  if (createInDir.exists(newFolderName)) {
    LOG_INFO_NP("[Skip] folder already exist", newFolderName);
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds{ACMD::GetInstMKPATH(createIn, newFolderName)};
  if (!UndoRedo::GetInst().Do(cmds)) {
    LOG_WARN_P("[Error] Make path failed", "%s/%s", qPrintable(createIn), qPrintable(newFolderName));
    return false;
  }
  if (folderAbsPath != nullptr) {
    *folderAbsPath = createInDir.absoluteFilePath(newFolderName);
  }
  return true;
}

bool NewItems(const QString& createIn, const QString& namePattern, int numStartIndex, int numEndIndex, bool isFolder) {
  if (numStartIndex >= numEndIndex) {
    LOG_WARN_P("[Abort] invalid range", "[%d, %d)", numStartIndex, numEndIndex);
    return false;
  }

  char namePatternArray[260 + 1] = {0};
  QByteArray utf8Data = namePattern.toUtf8(); // 转换为UTF-8编码[1,3](@ref)
  const int copySize = qMin(utf8Data.size(), 260);
  strncpy(namePatternArray, utf8Data.constData(), copySize);

  const QDir createInDir {createIn};
  using namespace FileOperatorType;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  for (int itemIndex = numStartIndex; itemIndex < numEndIndex; ++itemIndex) {
    QString fileNameArray = QString::asprintf(namePatternArray, itemIndex);
    if (createInDir.exists(fileNameArray)) {
      LOG_I("[Skip create]. File/Folder[%s] already exists in folder[%s]", qPrintable(fileNameArray), qPrintable(createIn));
      continue;
    }
    if (isFolder) {
      cmds.append(ACMD::GetInstMKPATH(createIn, fileNameArray));
    } else {
      cmds.append(ACMD::GetInstTOUCH(createIn, fileNameArray));
    }
  }
  if (!UndoRedo::GetInst().Do(cmds)) {
    LOG_WARN_P("[Error] Some commands failed",  "commands count: %d", cmds.size());
    return false;
  }
  return true;;
}


}
