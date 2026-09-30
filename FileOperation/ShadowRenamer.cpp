#include "ShadowRenamer.h"
#include "FileOperatorPub.h"
#include "UndoRedo.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "Logger.h"
#include <QFileInfo>
#include <QDirIterator>
namespace ShadowRenamer {

bool CheckPathValid(const QString &folderFullPathContainsVideosNeedRename, const QString &shadowPath) {
  if (!QFileInfo{folderFullPathContainsVideosNeedRename}.isDir()) {
    LOG_W("Source path [%s] does not exist", qPrintable(folderFullPathContainsVideosNeedRename));
    return false;
  }

  if (!QFileInfo{shadowPath}.isDir()) {
    LOG_W("Destination path [%s] does not exist", qPrintable(shadowPath));
    return false;
  }

  if (!QDir{shadowPath}.isEmpty()) {
    LOG_W("Destination path [%s] is not empty folder", qPrintable(shadowPath));
    return false;
  }
  // folderFullPathContainsVideosNeedRename: "C:/home/HelloWorld"
  // shadowPath: "C:/home/Hello" return true valid
  // shadowPath: "C:/home/HelloWorld" return false invalid
  // shadowPath: "C:/home/HelloWorldOther" return true valid
  // shadowPath: "C:/home/HelloWorld/sub" return false invalid
  const QString src{folderFullPathContainsVideosNeedRename + '/'};
  const QString dst{shadowPath + '/'};
  if (dst.startsWith(src)) {
    LOG_W("Destination path [%s] cannot be a subfolder of source path [%s]", qPrintable(shadowPath), qPrintable(folderFullPathContainsVideosNeedRename));
    return false;
  }
  return true;
}

std::pair<QString, SHADOW_STATUS> ParseShadowFileContent(const QString& shadowFileContent) {
  const int tableCharIndex = shadowFileContent.indexOf('\t');
  if (tableCharIndex == -1) {
    return {"", SHADOW_STATUS::NO_CORRESPOND_FILE};
  }
  const QString statusStr = shadowFileContent.mid(tableCharIndex + 1);
  bool bValidStatus{false};
  const int statusInt{statusStr.toInt(&bValidStatus)};
  if (!bValidStatus) {
    return {"", SHADOW_STATUS::NO_CORRESPOND_FILE};
  }
  if (!isShadowStatusValid(statusInt)) {
    return {"", SHADOW_STATUS::NO_CORRESPOND_FILE};
  }
  return {shadowFileContent.left(tableCharIndex), static_cast<SHADOW_STATUS>(statusInt)};
}

// dir="C:/home", relative2File="X-Men Packs/Michael Fassbender.mp4"
// prepare parent path "X-Men Packs" required
bool PrepareForParentFolder(QDir& dir, const QString& relative2File) {
  const int lastIndexOfSlash = relative2File.lastIndexOf('/');
  if (lastIndexOfSlash == -1) {
    return true;
  }
  const QString newSrcFileLocatedIn = relative2File.left(lastIndexOfSlash);
  return dir.exists(newSrcFileLocatedIn) || dir.mkpath(newSrcFileLocatedIn);
}

int CreateShadowFileUsingNameAsContent(const QString &folderFullPathContainsVideosNeedRename, const QString &shadowPath) {
  if (!CheckPathValid(folderFullPathContainsVideosNeedRename, shadowPath)) {
    return -1;
  }
  int shadowFilesCnt{0};
  const int prePathLen = folderFullPathContainsVideosNeedRename.size() + 1;
  QDirIterator it{folderFullPathContainsVideosNeedRename, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QDir shadowPathDir{shadowPath};
  while (it.hasNext()) {
    const QString fileFullPath = it.next();
    const QString relativePath2File = fileFullPath.mid(prePathLen);
    const QString destFileFullPath = PathTool::Path2Join(shadowPath, relativePath2File);
    const QString contentAlsoFileName = JoinShadowFileContent(relativePath2File, SHADOW_STATUS::PENDING_SYNC);
    if (!PrepareForParentFolder(shadowPathDir, relativePath2File)) {
      LOG_W("mkpath parent path for Shadow file[%s] failed", qPrintable(destFileFullPath));
      return -1;
    }
    if (!FileTool::StringTextWriter(destFileFullPath, contentAlsoFileName, QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)) {
      LOG_W("Shadow file[%s] create failed", qPrintable(destFileFullPath));
      return -1;
    }
    ++shadowFilesCnt;
  }
  LOG_D("%d shadow file(s) of videos under[%s] created into [%s] succeed", shadowFilesCnt, qPrintable(folderFullPathContainsVideosNeedRename), qPrintable(shadowPath));
  return shadowFilesCnt;
}

/* 状态流转图

创建影子 CreateShadowFileUsingNameAsContent
   │
   ▼
PENDING_SYNC ──────用户改影子──────▶ 路径 != 内容 ──▶ 回写源文件
   │                                                   │
   │                                          succeed  │  failed
   │                                                   ▼    ▼
   │                                              ALREADY_SYNC(final)
   │                                                   │
   │                                              写影子失败
   │                                                   ▼
   │                                          （下次幂等恢复）
   │
用户没改影子
   │
   ▼
路径 == 内容 ──▶ NO_NEED_SYNC（暂态）
                    │
                    │ 用户后来又改影子
                    ▼
              路径 != 内容 ──▶ 回写源文件

*/

bool IsTreatAsShadowFile(const QString& shadowFileFullPath) {
   // 260 char(s) * 4 bytes each UTF8
  return QFile{shadowFileFullPath}.size() < 260 * 4;
}
std::pair<bool, SyncDetails> SyncSourceFileByShadowFile(const QString &shadowPath, const QString &folderFullPathContainsVideosNeedRename) {
  SyncDetails detail;
  QString oldSrcVidFileName; // relative
  SHADOW_STATUS shadowFileStatus{SHADOW_STATUS::NO_CORRESPOND_FILE};

  QDir oldPathDir{folderFullPathContainsVideosNeedRename};
  const int prePathLen = shadowPath.size() + 1;
  QDirIterator it{shadowPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    const QString shadowFileFullPath = it.next();
    bool bReadOk{false};
    if (!IsTreatAsShadowFile(shadowFileFullPath)) {
      LOG_W("File is not shadow[%s]", qPrintable(shadowFileFullPath));
      continue;
    }
    const QString oldContentAlsoFileName = FileTool::StringTextReader(shadowFileFullPath, &bReadOk);
    if (!bReadOk) {
      LOG_W("Shadow file[%s] read failed", qPrintable(shadowFileFullPath));
      return {false, detail};
    }
    std::tie(oldSrcVidFileName, shadowFileStatus) = ParseShadowFileContent(oldContentAlsoFileName);
    if (oldSrcVidFileName.isEmpty()) {
      LOG_W("Shadow file[%s] content[%s] is broken", qPrintable(shadowFileFullPath), qPrintable(oldContentAlsoFileName));
      return {false, detail};
    }
    ++detail.shadowFilesCnt;
    if (shadowFileStatus == SHADOW_STATUS::ALREADY_SYNCED) {
      ++detail.alreadySyncedCnt;
      continue;
    }
    QString newContentAlsoFileName;
    const QString newSrcVidFileName = shadowFileFullPath.mid(prePathLen);
    if (newSrcVidFileName == oldSrcVidFileName) {
      // 影子文件所在的相对路径 和 内容路径部分一致 ->原文件名称无需同步
      newContentAlsoFileName = JoinShadowFileContent(oldSrcVidFileName, SHADOW_STATUS::NO_NEED_SYNC);
      ++detail.noNeedSyncCnt;
    } else {
      const QString oldSrcVidFileFullPath = PathTool::Path2Join(folderFullPathContainsVideosNeedRename, oldSrcVidFileName);
      const QString newSrcVidFileFullPath = PathTool::Path2Join(folderFullPathContainsVideosNeedRename, newSrcVidFileName);
      const bool bOldSrcExists = QFileInfo(oldSrcVidFileFullPath).isFile();
      const bool bNewSrcExists = QFileInfo(newSrcVidFileFullPath).isFile();

      if (!bOldSrcExists && bNewSrcExists) {
        // 异常处理: 幂等恢复 旧文件不存在，但新文件已存在 -> 之前已经重命名成功 但是影子文件更新失败
        LOG_D("Source video [%s] already renamed to [%s], mark shadow as ALREADY_SYNC", qPrintable(oldSrcVidFileFullPath), qPrintable(newSrcVidFileFullPath));
        newContentAlsoFileName = JoinShadowFileContent(newSrcVidFileName, SHADOW_STATUS::ALREADY_SYNCED);
        ++detail.alreadySyncedCnt;
      } else if (!bOldSrcExists && !bNewSrcExists) {
        // 旧文件和新文件都不存在->源文件丢失。
        LOG_W("No source video correspond to shadow file[%s]", qPrintable(oldSrcVidFileFullPath));
        newContentAlsoFileName = JoinShadowFileContent(oldSrcVidFileName, SHADOW_STATUS::NO_CORRESPOND_FILE);
        ++detail.noCorrespondingCnt;
      } else {
        // 准备前置路径 同步原文件名称和路径结构为影子文件名称和路径结构
        if (!PrepareForParentFolder(oldPathDir, newSrcVidFileName)) {
          LOG_W("mkpath parent path for new video file[%s] failed", qPrintable(newSrcVidFileName));
          return {false, detail};
        }
        if (QFile::rename(oldSrcVidFileFullPath, newSrcVidFileFullPath)) {
          newContentAlsoFileName = JoinShadowFileContent(newSrcVidFileName, SHADOW_STATUS::ALREADY_SYNCED);
          ++detail.newSyncedCnt;
        } else {
          ++detail.pendingSyncCnt;
          newContentAlsoFileName = JoinShadowFileContent(oldSrcVidFileName, SHADOW_STATUS::PENDING_SYNC);
          LOG_W("Rename Src video file[%s->%s] failed", qPrintable(oldSrcVidFileFullPath), qPrintable(newSrcVidFileFullPath));
        }
      }
    }
    if (newContentAlsoFileName == oldContentAlsoFileName) {
      continue;
    }
    if (!FileTool::StringTextWriter(shadowFileFullPath, newContentAlsoFileName, QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)) {
      LOG_W("Shadow file[%s] create failed", qPrintable(shadowFileFullPath));
      return {false, detail};
    }
  }
  LOG_D("%s", qPrintable(detail.logStr()));
  return {true, detail};
}

int onCreateStagingFile(const QString& srcPath) {
  const QString stagingFolder = GetStagingFileFolder(srcPath);
  if (!QFile::exists(stagingFolder) && !QDir{}.mkpath(stagingFolder)) {
    LOG_W("Cannot mkpath[%s] for stashing folder failed", qPrintable(stagingFolder));
    return -1;
  }
  return CreateShadowFileUsingNameAsContent(srcPath, stagingFolder);
}

SyncDetails onShowStagingStatistics(const QString& shadowPath) {
  if (!isStagingFileFolder(shadowPath)) {
    LOG_W("Path[%s] not match shadow pattern", qPrintable(shadowPath));
    return SyncDetails{};
  }
  QStringList pendingSyncList, alreadySyncedList, noNeedSyncList, noCorrespondList;
  int parsedCount{0};

  QString oldSrcVidFileName; // relative
  SHADOW_STATUS shadowFileStatus{SHADOW_STATUS::NO_CORRESPOND_FILE};

  const int prePathLen = shadowPath.size() + 1;
  QDirIterator it{shadowPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    const QString shadowFileFullPath = it.next();
    if (!IsTreatAsShadowFile(shadowFileFullPath)) {
      LOG_W("File is not shadow[%s]", qPrintable(shadowFileFullPath));
      continue;
    }
    bool bReadOk{false};
    const QString content = FileTool::StringTextReader(shadowFileFullPath, &bReadOk);
    if (!bReadOk) {
      LOG_W("Shadow file [%s] read failed", qPrintable(shadowFileFullPath));
      continue;
    }
    std::tie(oldSrcVidFileName, shadowFileStatus) = ParseShadowFileContent(content);
    if (oldSrcVidFileName.isEmpty()) {
      LOG_W("Shadow file [%s] content [%s] is broken", qPrintable(shadowFileFullPath), qPrintable(content));
      continue;
    }
    ++parsedCount;
    // relativePath2ShadowFile \t FileContent
    const QString new2Old = shadowFileFullPath.mid(prePathLen) + '\t' + oldSrcVidFileName;
    switch (shadowFileStatus) {
      case SHADOW_STATUS::PENDING_SYNC:
        pendingSyncList.append(new2Old);
        break;
      case SHADOW_STATUS::ALREADY_SYNCED:
        alreadySyncedList.append(new2Old);
        break;
      case SHADOW_STATUS::NO_NEED_SYNC:
        noNeedSyncList.append(new2Old);
        break;
      case SHADOW_STATUS::NO_CORRESPOND_FILE:
        noCorrespondList.append(new2Old);
        break;
      default:
        LOG_W("Shadow file [%s] has unknown status %d", qPrintable(shadowFileFullPath), (int)shadowFileStatus);
        break;
    }
  }
  QString logStr;
  logStr += '\n';
  logStr += QString("[PENDING_SYNC] %1 file(s) as follows:\n").arg(pendingSyncList.size());
  logStr += pendingSyncList.join('\n');
  logStr += '\n';
  logStr += QString("[ALREADY_SYNCED] %1 file(s) as follows:\n").arg(alreadySyncedList.size());
  logStr += alreadySyncedList.join('\n');
  logStr += '\n';
  logStr += QString("[NO_NEED_SYNC] %1 file(s) as follows:\n").arg(noNeedSyncList.size());
  logStr += noNeedSyncList.join('\n');
  logStr += '\n';
  logStr += QString("[NO_CORRESPOND_FILE] %1 file(s) as follows:\n").arg(noCorrespondList.size());
  logStr += noCorrespondList.join('\n');
  logStr += '\n';
  LOG_W("%s", qPrintable(logStr));
  return SyncDetails{parsedCount, pendingSyncList.size(), alreadySyncedList.size(), noNeedSyncList.size(), noCorrespondList.size(), 0};
}

std::pair<bool, int> recycleStagingFileWithGivenStatus(const QString& shadowPath, int removeStatusBits) {
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;

  QString oldSrcVidFileName; // relative
  SHADOW_STATUS shadowFileStatus{SHADOW_STATUS::NO_CORRESPOND_FILE};

  QDirIterator it{shadowPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    const QString shadowFileFullPath = it.next();
    if (!IsTreatAsShadowFile(shadowFileFullPath)) {
      LOG_W("File is not shadow[%s]", qPrintable(shadowFileFullPath));
      continue;
    }
    bool bReadOk{false};
    const QString content = FileTool::StringTextReader(shadowFileFullPath, &bReadOk);
    if (!bReadOk) {
      LOG_W("Shadow file[%s] read failed", qPrintable(shadowFileFullPath));
      continue;
    }

    std::tie(oldSrcVidFileName, shadowFileStatus) = ParseShadowFileContent(content);
    if (oldSrcVidFileName.isEmpty()) {
      LOG_W("Shadow file[%s] content[%s] is broken", qPrintable(shadowFileFullPath), qPrintable(content));
      continue;
    }

    if (!IsStatusInRemoveBits(removeStatusBits, shadowFileStatus)) {
      continue;
    }
    recycleCmds.append(ACMD::GetInstMOVETOTRASH("", shadowFileFullPath));
  }
  if (recycleCmds.isEmpty()) {
    return {true, 0};
  }
  LOG_D("Deleted %d staging file(s) with status bits [%d]", recycleCmds.size(), removeStatusBits);
  bool isRecycleAllSucceed = UndoRedo::GetInst().Do(recycleCmds);
  return {isRecycleAllSucceed, recycleCmds.size()};
}

std::pair<bool, int> onRecycleNoNeedSyncStagingFile(const QString& shadowPath) {
  if (!isStagingFileFolder(shadowPath)) {
    LOG_W("Path[%s] not match shadow pattern", qPrintable(shadowPath));
    return {false, -1};
  }
  return recycleStagingFileWithGivenStatus(shadowPath, (int)SHADOW_STATUS::NO_NEED_SYNC);
}
std::pair<bool, int> onRecycleAlreadySyncedStagingFile(const QString& shadowPath) {
  if (!isStagingFileFolder(shadowPath)) {
    LOG_W("Path[%s] not match shadow pattern", qPrintable(shadowPath));
    return {false, -1};
  }
  return recycleStagingFileWithGivenStatus(shadowPath, (int)SHADOW_STATUS::ALREADY_SYNCED);
}

}