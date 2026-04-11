#include "BatchRenameBy.h"
#include "PathTool.h"
#include "RenameWidget_Replace.h"
#include "RenameWidget_Insert.h"
#include "RenameWidget_Numerize.h"
#include "NotificatorMacro.h"
#include <QDir>

namespace BatchRenameBy {
static const QRegularExpression JSON_RELATED_FILE_BASENAME_PATTERN{"^( | - )(\\d{1,3})$"};

QStringList GetFilesNeedRename(const QString& path, const QStringList& jsonNames) {
  QStringList filesNeedRename;
  QString relPath;
  QString jsonFileName;
  QString jsonBaseName, jsonExt;

  QString fileBaseName, fileExt;

  QString jsonLevelPath;
  jsonLevelPath.reserve(256);

  QHash<QString, QStringList> dirToJsonNames;

  QDir sameLevelDir{path, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  for (const QString& rel2Json : jsonNames) {
    jsonFileName = PathTool::GetPrepathAndFileName(rel2Json, relPath);
    const bool isRel{!relPath.isEmpty()};
    jsonLevelPath = path;
    if (isRel) {
      jsonLevelPath += '/';
      jsonLevelPath += relPath;
    }
    auto relIt = dirToJsonNames.find(jsonLevelPath);
    if (relIt == dirToJsonNames.end()) {
      if (sameLevelDir.path() != jsonLevelPath) {
        sameLevelDir.setPath(jsonLevelPath);
      }
      relIt = dirToJsonNames.insert(jsonLevelPath, sameLevelDir.entryList());
    }

    std::tie(jsonBaseName, jsonExt) = PathTool::GetBaseNameExt(jsonFileName);
    foreach (const QString& fileName, relIt.value()) {
      if (!fileName.startsWith(jsonBaseName)) {
        continue;
      }
      std::tie(fileBaseName, fileExt) = PathTool::GetBaseNameExt(fileName);
      // fileBaseName == (jsonBaseName + extraContent) + fileExt
      // extraContent can be
      // 1. "": (fileBaseName == jsonBaseName)
      // 2. " - number": len(fileBaseName) > len(jsonBaseName)
      // 3. " number": len(fileBaseName) > len(jsonBaseName)
      QString extraContent = fileBaseName.mid(jsonBaseName.size());
      if (!extraContent.isEmpty() && !JSON_RELATED_FILE_BASENAME_PATTERN.match(extraContent).hasMatch()) {
        continue; // Todo: llt cover this line
      }
      filesNeedRename.push_back(isRel ? relPath + '/' + fileName : fileName);
    }
  }
  std::sort(filesNeedRename.begin(), filesNeedRename.end());
  auto dupStartIt = std::unique(filesNeedRename.begin(), filesNeedRename.end());
  filesNeedRename.erase(dupStartIt, filesNeedRename.end());
  return filesNeedRename;
}

RnmResult InsertQueryAndConfirm(const QString& workPath,          //
                                const QStringList& selectedNames, //
                                const QString& defStrInsert,      //
                                const int insertAtIndex) {
  if (selectedNames.isEmpty()) {
    return RnmResult::SKIP;
  }
  RenameWidget_Insert pInserter{nullptr};
  pInserter.init();
  pInserter.setModal(true);
  pInserter.InitTextEditContent(workPath, selectedNames);
  pInserter.setStrInsertAndIndex(defStrInsert, insertAtIndex);
  if (RenameWidget_Replace::execCore(&pInserter) != QDialog::DialogCode::Accepted) {
    LOG_D("Skip, user cancel insert rename %d item(s)", selectedNames.size());
    return RnmResult::SKIP;
  }
  return pInserter.GetApplyResult() ? RnmResult::ALL_SUCCEED : RnmResult::PARTIAL_FAILED;
}

RnmResult ReplaceQueryAndConfirm(const QString& workPath,          //
                                 const QStringList& selectedNames, //
                                 const QString& defOldName,        //
                                 const QString& defNewName,        //
                                 bool disableOldNameEdit) {
  if (selectedNames.isEmpty()) {
    return RnmResult::SKIP;
  }
  RenameWidget_Replace pReplacer{nullptr};
  pReplacer.init();
  pReplacer.setModal(true);
  pReplacer.InitTextEditContent(workPath, selectedNames);
  pReplacer.setOldNameAndNewName(defOldName, defNewName);
  pReplacer.setOldLineEditDisabled(disableOldNameEdit);

  if (RenameWidget_Replace::execCore(&pReplacer) != QDialog::DialogCode::Accepted) {
    LOG_D("Skip, user cancel replace rename %d item(s)", selectedNames.size());
    return RnmResult::SKIP;
  }
  return pReplacer.GetApplyResult() ? RnmResult::ALL_SUCCEED : RnmResult::PARTIAL_FAILED;
}

RnmResult NumerizerQueryAndConfirm(const QString& workPath, //
                                   const QStringList& selectedNames) {
  if (selectedNames.isEmpty()) {
    return RnmResult::SKIP;
  }
  RenameWidget_Numerize pNumerize{nullptr};
  pNumerize.init();
  pNumerize.setModal(true);
  pNumerize.InitTextEditContent(workPath, selectedNames);

  if (RenameWidget_Numerize::execCore(&pNumerize) != QDialog::DialogCode::Accepted) {
    LOG_D("Skip, user cancel numerize rename %d item(s)", selectedNames.size());
    return RnmResult::SKIP;
  }
  return pNumerize.GetApplyResult() ? RnmResult::ALL_SUCCEED : RnmResult::PARTIAL_FAILED;
}

int InsertBySpecifiedJson(const QString& path, const QStringList& jsonNames) {
  const QStringList& filesNeedRename{GetFilesNeedRename(path, jsonNames)};
  if (filesNeedRename.isEmpty()) {
    return 0;
  }
  const RnmResult rnmResult{InsertQueryAndConfirm(path, filesNeedRename)};
  if (rnmResult == RnmResult::SKIP) {
    LOG_INFO_P("[Skip] rename", "User cancel rename %d item(s)", filesNeedRename.size());
    return 0;
  }
  LOG_OE_P(rnmResult == RnmResult::ALL_SUCCEED,
           "Rename(Insert) json/video related", //
           "%d item(s) specified by %d pattern renamed",
           filesNeedRename.size(),
           jsonNames.size());
  return filesNeedRename.size();
}

QString GetDefaultOldName(const QStringList& jsonNames, bool& bDisableOldNameLineEdit) {
  bDisableOldNameLineEdit = jsonNames.size() == 1;
  QString defOldName;
  if (bDisableOldNameLineEdit) {
    QString jsonBaseName, _;
    std::tie(jsonBaseName, _) = PathTool::GetBaseNameExt(jsonNames.front());
    defOldName.swap(jsonBaseName);
  }
  return defOldName;
}

int ReplaceBySpecifiedJson(const QString& path, const QStringList& jsonNames) {
  const QStringList& filesNeedRename{GetFilesNeedRename(path, jsonNames)};
  if (filesNeedRename.isEmpty()) {
    return 0;
  }
  bool bDisableOldNameLineEdit{false};
  const QString defOldName{GetDefaultOldName(jsonNames, bDisableOldNameLineEdit)};
  const RnmResult rnmResult{ReplaceQueryAndConfirm(path, filesNeedRename, defOldName, defOldName, bDisableOldNameLineEdit)};
  if (rnmResult == RnmResult::SKIP) {
    LOG_INFO_P("[Skip] rename", "User cancel rename %d item(s)", filesNeedRename.size());
    return 0;
  }
  LOG_OE_P(rnmResult == RnmResult::ALL_SUCCEED,
           "Rename(Replace) json/video related", //
           "%d item(s) specified by %d pattern renamed",
           filesNeedRename.size(),
           jsonNames.size());
  return filesNeedRename.size();
}

int NumerizerBySpecifiedJson(const QString& path, const QStringList& jsonNames) {
  const QStringList& filesNeedRename{GetFilesNeedRename(path, jsonNames)};
  if (filesNeedRename.isEmpty()) {
    return 0;
  }
  const RnmResult rnmResult{NumerizerQueryAndConfirm(path, filesNeedRename)};
  if (rnmResult == RnmResult::SKIP) {
    LOG_INFO_P("[Skip] rename", "User cancel rename %d item(s)", filesNeedRename.size());
    return 0;
  }
  LOG_OE_P(rnmResult == RnmResult::ALL_SUCCEED,
           "Rename(Numerize) json/video related", //
           "%d item(s) specified by %d pattern renamed",
           filesNeedRename.size(),
           jsonNames.size());
  return filesNeedRename.size();
}

} // namespace BatchRenameBy
