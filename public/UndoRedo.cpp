#include "UndoRedo.h"
#include "SyncModifiyFileSystem.h"
#include "FileOperation.h"
using namespace FileOperatorType;

UndoRedo& UndoRedo::GetInst() {
  static UndoRedo undoRedo;
  return undoRedo;
}

BATCH_COMMAND_LIST_TYPE syncExecuterconst(const BATCH_COMMAND_LIST_TYPE& aBatch) {
  if (!SyncModifiyFileSystem::m_syncOperationSw) {
    return {};
  }
  BATCH_COMMAND_LIST_TYPE syncBatch;
  syncBatch.reserve(aBatch.size());
  const SyncModifiyFileSystem syncMod;
  for (int i = 0; i < aBatch.size(); ++i) {
    ACMD cmds = aBatch[i];
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
  if (SyncModifiyFileSystem::m_syncOperationSw) {
    syncCmd = syncExecuterconst(cmd);
    if (!syncCmd.isEmpty()) {
      const auto& syncRetEle = FileOperation::executer(syncCmd);
      if (!syncRetEle) {
        LOG_W("sync commands failed");
      }
      exeRetEle.ret = (exeRetEle.ret == ErrorCode::OK && syncRetEle.ret == ErrorCode::OK ? ErrorCode::OK : ErrorCode::UNKNOWN_ERROR);
      exeRetEle.cmds += syncRetEle.cmds;
    }
  }
  mUndoStk.append(exeRetEle.cmds);
  return (bool)exeRetEle;
}

bool UndoRedo::on_Undo() { // for undo action
  if (!GetInst().undoAvailable()) {
    LOG_I("[skip] Nothing to undo");
    return true;
  }
  const bool isAllSucceed = GetInst().Undo();
  const char* undoMsg = isAllSucceed ? "All undo succeed" : "Some undo failed.";
  LOG_D("%s", undoMsg);
  return isAllSucceed;
}

bool UndoRedo::on_Redo() {  // for undo action
  if (!GetInst().redoAvailable()) {
    LOG_I("[skip] Nothing to redo");
    return true;
  }
  const bool isAllSucceed = GetInst().Redo();
  const char* redoMsg = isAllSucceed ? "All redo succeed" : "Some redo failed.";
  LOG_D("%s", redoMsg);
  return isAllSucceed;
}

bool UndoRedo::Undo() {
  if (!undoAvailable()) {
    LOG_D("Skip. Cannot undo");
    return true;
  }
  BATCH_COMMAND_LIST_TYPE& undoCmds = mUndoStk.top();
  std::reverse(undoCmds.begin(), undoCmds.end());
  const RETURN_TYPE& redoEles = FileOperation::executer(undoCmds);
  mUndoStk.pop();
  mRedoStk.append(std::move(redoEles.cmds));
  return (bool)redoEles;
}

bool UndoRedo::Redo() {
  if (!redoAvailable()) {
    LOG_D("Skip. Cannot redo");
    return true;
  }
  BATCH_COMMAND_LIST_TYPE& redoCmds = mRedoStk.top();
  std::reverse(redoCmds.begin(), redoCmds.end());
  const RETURN_TYPE& undoEles = FileOperation::executer(redoCmds);
  mRedoStk.pop();
  mUndoStk.append(std::move(undoEles.cmds));
  return (bool)undoEles;
}
