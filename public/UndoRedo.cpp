#include "UndoRedo.h"
#include "Component/SyncModifiyFileSystem.h"
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
  BATCH_COMMAND_LIST_TYPE srcCommand;
  auto exeRetEle = FileOperation::executer(cmd, srcCommand);

  BATCH_COMMAND_LIST_TYPE syncCmd;
  if (SyncModifiyFileSystem::m_syncModifyFileSystemSwitch) {
    syncCmd = syncExecuterconst(cmd);
    if (!syncCmd.isEmpty()) {
      BATCH_COMMAND_LIST_TYPE syncSrcCommand;
      const auto& syncRetEle = FileOperation::executer(syncCmd, syncSrcCommand);
      if (!syncRetEle) {
        qWarning("sync commands failed");
      }
      exeRetEle.ret = exeRetEle.ret && syncRetEle.ret;
      exeRetEle.cmds = syncRetEle.cmds + exeRetEle.cmds;
    }
  }
  undoList.append(OperationStream{cmd + syncCmd, exeRetEle.cmds});
  return exeRetEle;
}

bool UndoRedo::on_Undo() {
  if (not g_undoRedo.undoAvailable()) {
    qInfo("[skip] Nothing to undo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Undo().first;
  const char* undoMsg = isAllSucceed ? "All undo succeed" : "Some undo failed.";
  qDebug("%s", undoMsg);
  return isAllSucceed;
}

bool UndoRedo::on_Redo() {
  if (not g_undoRedo.redoAvailable()) {
    qInfo("[skip] Nothing to redo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Redo().first;
  const char* redoMsg = isAllSucceed ? "All redo succeed" : "Some redo failed.";
  qDebug("%s", redoMsg);
  return isAllSucceed;
}

UndoRedo::UNDO_REDO_RETURN UndoRedo::Undo() {
  if (not undoAvailable()) {
    qDebug("Skip Cannot undo");
    return qMakePair<bool, OperationStream>(true, OperationStream());
  }
  OperationStream ele = undoList.pop();
  const auto& exeRetEle = FileOperation::executer(ele.recoverCmd, ele.doCmd);
  redoList.append(ele);  // you can not just update do_cmd in ele. Because it is not equivelant as rmfile has no recover
  // compromise method: when pass the do_cmd list into it
  return qMakePair<bool, OperationStream>(exeRetEle, ele);
}

UndoRedo::UNDO_REDO_RETURN UndoRedo::Redo() {
  if (not redoAvailable()) {
    qDebug("Skip Cannot redo");
    return qMakePair<bool, OperationStream>(true, OperationStream());
  }
  OperationStream ele = redoList.pop();
  const auto& exeRetEle = FileOperation::executer(ele.doCmd, ele.recoverCmd);
  undoList.append(ele);
  return qMakePair<bool, OperationStream>(exeRetEle, ele);
}
