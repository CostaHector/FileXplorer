#include "FileBatchOps.h"
#include "PathTool.h"
#include "RenameWidget_Replace.h"
#include "RenameWidget_Insert.h"
#include "RenameWidget_Numerize.h"
#include "FileOpActs.h"
#include "FileOperatorPub.h"
#include "UndoRedo.h"
#include "ItemsPileCategory.h"
#include "NotificatorMacro.h"
#include "RecycleCfmDlg.h"
#include <QDir>

namespace FileBatchOps {
/*
 * relPatternNames 允许相对路径, 此时模式为baseName
Given: path=".../replace"
Given: jsonNames={"pattern/Chris Evans.json", "pattern/Jensen Ackles.mp4"}, can be json, mp4, pson, etc.
Given: json mp4 jpg png files exist and are under ".../replace/pattern".
Return:
        "pattern/Chris Evans 2.png",        //
        "pattern/Chris Evans.jpg",          //
        "pattern/Chris Evans.json",         //
        "pattern/Chris Evans_tn.jpg",       //
        "pattern/Jensen Ackles.mp4",        //
        "pattern/Jensen Ackles.pson",       //
*/
QStringList ExpandRelatedFiles(const QString& path, const QStringList& relPatternNames, QMap<QString, QString>* pFile2Pattern) {
  QStringList filesNeedRename;
  QString relPath;
  QString jsonFileName;
  QString jsonBaseName, jsonExt;

  QString fileBaseName, fileExt;

  QString jsonLevelPath;
  jsonLevelPath.reserve(256);

  QHash<QString, QStringList> dirToJsonNames;

  const auto prefixLessThan = [](const QStringList::ConstIterator& nameIt, const QString& prefix)->bool {
    const QString& fileName = (*nameIt);
    const int cmpLength{qMin(prefix.size(), fileName.size())};
    if (const int cmp = QString::compare(fileName.left(cmpLength), prefix.left(cmpLength))) { // !=0
      return cmp < 0;
    }
    return fileName.size() < prefix.size();
  };

  QDir sameLevelDir{path, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  for (const QString& rel2Json : relPatternNames) {
    jsonFileName = PathTool::GetPrepathAndFileName(rel2Json, relPath);
    const bool isRel{!relPath.isEmpty()};
    jsonLevelPath = path;
    if (isRel) {
      jsonLevelPath += '/';
      jsonLevelPath += relPath;
    }

    std::tie(jsonBaseName, jsonExt) = PathTool::GetBaseNameExt(jsonFileName);
    auto relIt = dirToJsonNames.find(jsonLevelPath);
    if (relIt == dirToJsonNames.end()) {
      if (sameLevelDir.path() != jsonLevelPath) {
        sameLevelDir.setPath(jsonLevelPath);
      }
      relIt = dirToJsonNames.insert(jsonLevelPath, sameLevelDir.entryList());
    }

    const QStringList& files{relIt.value()};
    auto firstFileIt = std::__lower_bound(files.cbegin(), files.cend(), jsonBaseName, prefixLessThan);
    for (auto it = firstFileIt; it != files.end(); ++it) {
      const QString& fileName = *it;
      if (!fileName.startsWith(jsonBaseName)) {
        continue;
      }
      std::tie(fileBaseName, fileExt) = PathTool::GetBaseNameExt(fileName);
      // fileBaseName == (jsonBaseName + extraContent) + fileExt. extraContent can be:
      // 1. "": (fileBaseName == jsonBaseName)
      // 2. "_tn"
      // 3. " - number": len(fileBaseName) > len(jsonBaseName)
      // 4. " number": len(fileBaseName) > len(jsonBaseName)
      const QString extraContent = fileBaseName.mid(jsonBaseName.size());
      if (extraContent.isEmpty()) {
        filesNeedRename.push_back(isRel ? relPath + '/' + fileName : fileName);
        if (pFile2Pattern) (*pFile2Pattern)[filesNeedRename.back()] = rel2Json;
        continue;
      }
      if (extraContent == PathTool::THUMBNAIL_FILE_ABBR) {
        filesNeedRename.push_back(isRel ? relPath + '/' + fileName : fileName);
        if (pFile2Pattern) (*pFile2Pattern)[filesNeedRename.back()] = rel2Json;
        continue;
      }
      if (ItemsPileCategory::IMG_PILE_NAME_PATTERN.match(extraContent).hasMatch()) {
        filesNeedRename.push_back(isRel ? relPath + '/' + fileName : fileName);
        if (pFile2Pattern) (*pFile2Pattern)[filesNeedRename.back()] = rel2Json;
        continue;
      }
    }
  }
  std::sort(filesNeedRename.begin(), filesNeedRename.end());
  auto dupStartIt = std::unique(filesNeedRename.begin(), filesNeedRename.end());
  filesNeedRename.erase(dupStartIt, filesNeedRename.end());
  return filesNeedRename;
}

void prepareRenameDialog(AdvanceRenamer &widget, const QString &workPath, const QStringList &names, const BeforeInitTextContentFn &beforeInitTextFunc) {
  widget.init();
  widget.setModal(true);
  if (beforeInitTextFunc) {
    beforeInitTextFunc(widget);
  }
  widget.InitTextEditContent(workPath, names);
}

OpStatus execRenameDialog(AdvanceRenamer &widget) {
  if (AdvanceRenamer::execCore(&widget) != QDialog::Accepted) {
    return OpStatus::SKIP;
  }
  return widget.GetApplyResult() ? OpStatus::ALL_SUCCEED : OpStatus::PARTIAL_FAILED;
}

OpStatus RenameSelected(AdvanceRenamer &widget, const QString &workPath, const QStringList &selectedNames) {
  if (selectedNames.isEmpty()) {
    return OpStatus::SKIP;
  }
  prepareRenameDialog(widget, workPath, selectedNames);
  return execRenameDialog(widget);
}

OpOutcome RenameSelectedWithRelated(AdvanceRenamer& widget, const QString& workPath, const QStringList& selectedJsonOrVideoNames) {
  const QStringList filesNeedRename = ExpandRelatedFiles(workPath, selectedJsonOrVideoNames);
  if (filesNeedRename.isEmpty()) {
    return {OpStatus::SKIP, 0};
  }
  return {RenameSelected(widget, workPath, filesNeedRename), filesNeedRename.size()};
}

// ---- 模式指定
int RenameWithPatterns(AdvanceRenamer& renamer, const QString& path, const QStringList& jsonOrVidNames, const char* logTag) {
  const OpOutcome outcome = RenameSelectedWithRelated(renamer, path, jsonOrVidNames);
  if (outcome.result == OpStatus::SKIP) {
    LOG_INFO_P("[Skip] rename", "User cancel rename %d item(s)", outcome.count);
    return 0;
  }
  LOG_OE_P(outcome.result == OpStatus::ALL_SUCCEED, "Rename by Pattern", "[%s] %d item(s) specified by %d pattern get renamed", logTag, outcome.count, jsonOrVidNames.size());
  return outcome.result == OpStatus::PARTIAL_FAILED ? -1 : outcome.count;
}

int RenameWithPatternInsert(const QString& path, const QStringList& jsonOrVidNames) {
  RenameWidget_Insert widget{nullptr};
  return RenameWithPatterns(widget, path, jsonOrVidNames, "Insert");
}

int RenameWithPatternReplace(const QString& path, const QStringList& jsonOrVidNames) {
  RenameWidget_Replace widget{nullptr};
  return RenameWithPatterns(widget, path, jsonOrVidNames, "Replace");
}

int RenameWithPatternNumerizer(const QString& path, const QStringList& jsonOrVidNames) {
  RenameWidget_Numerize widget{nullptr};
  return RenameWithPatterns(widget, path, jsonOrVidNames, "Numerize");
}

// ---- 直接指定
OpStatus RenameSelectedInsert(const QString& workPath, const QStringList& selectedNames) {
  RenameWidget_Insert widget{nullptr};
  return RenameSelected(widget, workPath, selectedNames);
}
OpStatus RenameSelectedReplace(const QString& workPath, const QStringList& selectedNames) {
  RenameWidget_Replace widget{nullptr};
  return RenameSelected(widget, workPath, selectedNames);
}
OpStatus RenameSelectedNumerizer(const QString& workPath, const QStringList& selectedNames) {
  RenameWidget_Numerize widget{nullptr};
  return RenameSelected(widget, workPath, selectedNames);
}

// ---- 回收相关核心入口
OpStatus MoveToTrashSelected(QString workPath, const QStringList &names) {
  if (names.isEmpty()) {
    LOG_INFO_NP("[Skip]", "Nothing selected");
    return OpStatus::SKIP;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(names.size());
  for (int i = 0; i < names.size(); ++i) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH(workPath, names[i]));
  }
  emit FileOpActs::GetInst().unlockOccupiedFiles();
  if (!UndoRedo::GetInst().Do(removeCmds)) {
    LOG_ERR_NP("[MoveToTrash] Partially failed", "Some item(s) move to trashbin failed");
    return OpStatus::PARTIAL_FAILED;
  }
  return OpStatus::ALL_SUCCEED;
}
OpStatus MoveToTrashSelectedWithRelated(QString workPath, const QStringList &selectedJsonOrVideoNames) {
  const QStringList filesNeedProcess = ExpandRelatedFiles(workPath, selectedJsonOrVideoNames);
  if (filesNeedProcess.isEmpty()) {
    LOG_INFO_NP("[Skip]", "Nothing to process after ExpandRelatedFiles");
    return OpStatus::SKIP;
  }
  if (!RecycleCfmDlg::recycleQuestion(workPath, filesNeedProcess, false)) {
    LOG_INFO_P("[Cancel] User cancel recycle", "%d expand to %d item(s) no change", selectedJsonOrVideoNames.size(), filesNeedProcess.size());
    return OpStatus::SKIP;
  }
  return MoveToTrashSelected(workPath, filesNeedProcess);
}

} // namespace BatchRenameBy
