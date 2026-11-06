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

  constexpr bool IGNORE_CASE{true};
  constexpr Qt::CaseSensitivity CASE_SENSITIVITY{IGNORE_CASE ? Qt::CaseInsensitive : Qt::CaseSensitive};
  constexpr QDir::SortFlags SORT_FLAG{IGNORE_CASE ? (QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase) : QDir::SortFlag::Name};
  const auto prefixLessThan = [](const QString& fileName, const QString& prefix) -> bool {
    const int cmpLength{qMin(prefix.size(), fileName.size())};
    if (const int cmp = QString::compare(fileName.left(cmpLength), prefix.left(cmpLength), CASE_SENSITIVITY)) {
      // [大小写不敏感控制 1], 通过模式"Michael Fassbender.mp4"可以额外找出path路径下的"michael fassbender.json"
      return cmp < 0;
    }
    return fileName.size() < prefix.size();
  };

  // [大小写不敏感控制 2] 获取路径下的文件/文件夹时, 保证有序且大小写不敏感
  QDir sameLevelDir{path, "", SORT_FLAG, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
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
    auto firstFileIt = std::lower_bound(files.cbegin(), files.cend(), jsonBaseName, prefixLessThan);
    for (auto it = firstFileIt; it != files.end(); ++it) {
      // file.mp4
      // file.jpg
      // file_tn.jpg
      // file 0.jpg
      // file - 0.jpg
      // file.mp4_20260818_155313.075.jpg
      // file_011050.075.jpg
      const QString& fileName = *it;
      // [大小写不敏感控制 3] 前缀只要大小写不敏感一致就行
      if (!fileName.startsWith(jsonBaseName, CASE_SENSITIVITY)) {
        continue;
      }
      std::tie(fileBaseName, fileExt) = PathTool::GetBaseNameExt(fileName);
      // fileBaseName == (jsonBaseName + extraContent) + fileExt.
      // extraContent can be:
      // 1. "":           (fileBaseName == jsonBaseName)
      // 2. "_tn"
      // 3.1 " - number": len(fileBaseName) > len(jsonBaseName)
      // 3.2 " number"  : len(fileBaseName) > len(jsonBaseName)
      // fileName can be endswith:
      // 4.1 "_20260818_155313.075.jpg"
      // 4.2 "_011050.075.jpg"
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
      if (ItemsPileCategory::GetImageNoTimeStampPatternLength(fileName) >= 0) {
        // 注意 mp4, mkv 会被同时扩展到
        // "file.mp4_20260818_155313.075.jpg" -> "file.mp4"
        // "file.mkv_20260818_155313.075.jpg" -> "file.mkv"
        // "file_011050.075.jpg"              -> "file"
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
namespace {
OpStatus MoveToTrashSelectedCore(const QStringList& workPaths, const QStringList &relNames) {
  if (relNames.isEmpty()) {
    LOG_INFO_NP("[Skip]", "Nothing selected");
    return OpStatus::SKIP;
  }
  // 允许两种输入:
  // 1. workPaths和relNames 元素一一匹配
  // 2. workPaths只有一个元素, 匹配到relNames的各个元素
  const bool isCommonWorkPath{workPaths.size() == 1};
  const bool isInputAllowed{workPaths.size() == relNames.size() || isCommonWorkPath};
  if (!isInputAllowed) {
    LOG_WARN_P("[Skip]", "Prepaths[%d] and Names[%d] length Inequal", workPaths.size(), relNames.size());
    return OpStatus::SKIP;
  }
  const QString commonWorkPathMsg{isCommonWorkPath ? workPaths.front() : "Working path varies..."};

  if (relNames.size() > 1 && !RecycleCfmDlg::recycleQuestion(commonWorkPathMsg, relNames, false)) {
    // show dialog only more than one file
    LOG_INFO_P("[Cancel] User cancel recycle", "%d item(s) no change", relNames.size());
    return OpStatus::SKIP;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(relNames.size());
  if (isCommonWorkPath) {
    const QString& commonWorkPath = workPaths.front();
    for (const QString& name : relNames) {
      removeCmds.append(ACMD::GetInstMOVETOTRASH(commonWorkPath, name));
    }
  } else {
    for (int i = 0; i < relNames.size(); ++i) {
      removeCmds.append(ACMD::GetInstMOVETOTRASH(workPaths[i], relNames[i]));
    }
  }

  emit FileOpActs::GetInst().unlockOccupiedFiles();
  if (!UndoRedo::GetInst().Do(removeCmds)) {
    LOG_ERR_NP("[MoveToTrash] Partially failed", "Some item(s) move to trashbin failed");
    return OpStatus::PARTIAL_FAILED;
  }
  return OpStatus::ALL_SUCCEED;
}
}

OpStatus MoveToTrashSelected(const QString& workPath, const QStringList &relNames) {
  // 用于FileSystemView, SceneView, JsonView, 这里的 names可以含前置相对路径的文件名(间接)
  return MoveToTrashSelectedCore({workPath}, relNames);
}

OpStatus MoveToTrashSelectedWithVaryRootPaths(const QStringList& workPaths, const QStringList &names) {
  // 用于 SearchView, MovieDBView, 这里的names是不含前置相对路径的文件名(直接)
  return MoveToTrashSelectedCore(workPaths, names);
}

OpOutcome MoveToTrashSelectedWithRelated(const QString& workPath, const QStringList &selectedJsonOrVideoNames) {
  const QStringList filesNeedProcess = ExpandRelatedFiles(workPath, selectedJsonOrVideoNames);
  if (filesNeedProcess.isEmpty()) {
    LOG_INFO_P("[Skip]", "Nothing to moveToTrash after ExpandRelatedFiles(%d) in %s", selectedJsonOrVideoNames.size(), qPrintable(workPath));
    return {OpStatus::SKIP, 0};
  }
  return {MoveToTrashSelected(workPath, filesNeedProcess), filesNeedProcess.size()};
}

} // namespace BatchRenameBy
