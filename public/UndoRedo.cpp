#include "UndoRedo.h"
#include "Tools/SyncModifiyFileSystem.h"
#include "FileOperation/FileOperation.h"
using namespace FileOperatorType;

UndoRedo g_undoRedo;

BATCH_COMMAND_LIST_TYPE syncExecuterconst(const BATCH_COMMAND_LIST_TYPE& aBatch) {
  if (!SyncModifiyFileSystem::m_syncModifyFileSystemSwitch) {
    return {};
  }
  BATCH_COMMAND_LIST_TYPE syncBatch;
  syncBatch.reserve(aBatch.size());
  const SyncModifiyFileSystem syncMod;
  for (int i = 0; i < aBatch.size(); ++i) {
    auto cmds = aBatch[i];
    if (!cmds) {
      continue;
    }
    bool cmdNeedSync{false};
    for (int parm = 0; parm < cmds.size(); ++parm) {
      if (syncMod(cmds[parm])) {
        cmdNeedSync = true;
      }
    }
    if (!cmdNeedSync) {
      continue;
    }
    syncBatch.append(cmds);
  }
  return syncBatch;
}

bool UndoRedo::Do(const BATCH_COMMAND_LIST_TYPE& cmd) {
  RETURN_TYPE exeRetEle = FileOperation::executer(cmd);

  BATCH_COMMAND_LIST_TYPE syncCmd;
  if (SyncModifiyFileSystem::m_syncModifyFileSystemSwitch) {
    syncCmd = syncExecuterconst(cmd);
    if (!syncCmd.isEmpty()) {
      const auto& syncRetEle = FileOperation::executer(syncCmd);
      if (!syncRetEle) {
        qWarning("sync commands failed");
      }
      exeRetEle.ret = (exeRetEle.ret == ErrorCode::OK && syncRetEle.ret == ErrorCode::OK ? ErrorCode::OK : ErrorCode::UNKNOWN_ERROR);
      exeRetEle.cmds = syncRetEle.cmds + exeRetEle.cmds;
    }
  }
  undoList.append(exeRetEle.cmds);
  return (bool)exeRetEle;
}

bool UndoRedo::on_Undo() {
  if (!g_undoRedo.undoAvailable()) {
    qInfo("[skip] Nothing to undo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Undo();
  const char* undoMsg = isAllSucceed ? "All undo succeed" : "Some undo failed.";
  qDebug("%s", undoMsg);
  return isAllSucceed;
}

bool UndoRedo::on_Redo() {
  if (!g_undoRedo.redoAvailable()) {
    qInfo("[skip] Nothing to redo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Redo();
  const char* redoMsg = isAllSucceed ? "All redo succeed" : "Some redo failed.";
  qDebug("%s", redoMsg);
  return isAllSucceed;
}

bool UndoRedo::Undo() {
  if (!undoAvailable()) {
    qDebug("Skip Cannot undo");
    return true;
  }
  const BATCH_COMMAND_LIST_TYPE& undoEles = undoList.top();
  const RETURN_TYPE& redoEles = FileOperation::executer(undoEles);
  undoList.pop();
  redoList.append(std::move(redoEles.cmds));  // you can not just update do_cmd in ele. Because it is not equivelant as rmfile has no recover
  // compromise method: when pass the do_cmd list into it
  return (bool)redoEles;
}

bool UndoRedo::Redo() {
  if (!redoAvailable()) {
    qDebug("Skip Cannot redo");
    return true;
  }
  const BATCH_COMMAND_LIST_TYPE& redoEles = redoList.top();
  const RETURN_TYPE& undoEles = FileOperation::executer(redoEles);
  redoList.pop();
  undoList.append(std::move(undoEles.cmds));
  return (bool)undoEles;
}
