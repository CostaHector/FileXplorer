#include "UndoRedo.h"
#include "Component/SyncModifiyFileSystem.h"
UndoRedo g_undoRedo;

auto syncExecuterconst(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& aBatch) -> FileOperatorType::BATCH_COMMAND_LIST_TYPE {
  if (!SyncModifiyFileSystem::m_syncModifyFileSystemSwitch) {
    return {};
  }
  FileOperatorType::BATCH_COMMAND_LIST_TYPE syncBatch;
  syncBatch.reserve(aBatch.size());
  const SyncModifiyFileSystem syncMod;
  for (int i = 0; i < aBatch.size(); ++i) {
    QStringList cmds = aBatch[i];
    if (cmds.isEmpty()) {
      continue;
    }
    bool cmdNeedSync{false};
    for (int parm = 1; parm < cmds.size(); ++parm) {
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

auto UndoRedo::Do(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmd) -> bool {
  FileOperatorType::BATCH_COMMAND_LIST_TYPE srcCommand;
  FileOperatorType::EXECUTE_RETURN_TYPE exeRetEle = FileOperation::executer(cmd, srcCommand);
  FileOperatorType::BATCH_COMMAND_LIST_TYPE syncCmd;
  if (SyncModifiyFileSystem::m_syncModifyFileSystemSwitch) {
    syncCmd = syncExecuterconst(cmd);
    if (!syncCmd.isEmpty()) {
      FileOperatorType::BATCH_COMMAND_LIST_TYPE syncSrcCommand;
      const FileOperatorType::EXECUTE_RETURN_TYPE& syncRetEle = FileOperation::executer(syncCmd, syncSrcCommand);
      if (!syncRetEle.first) {
        qWarning("sync commands failed");
      }
      exeRetEle.first = exeRetEle.first && syncRetEle.first;
      exeRetEle.second = syncRetEle.second + exeRetEle.second;
    }
  }
  const auto isAllSucceed = exeRetEle.first;
  const auto& recover_cmd = exeRetEle.second;
  undoList.append(OperationStream{cmd + syncCmd, recover_cmd});
  return isAllSucceed;
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
