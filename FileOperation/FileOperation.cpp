#include "FileOperation.h"
#include <QDateTime>

#include "RecycleBinHelper.h"

const QMap<QString, std::function<FileOperatorType::RETURN_TYPE(const QStringList&)>> FileOperation::LambdaTable = {
    {"rmfile", rmfileAgent}, {"rmpath", rmpathAgent}, {"rmdir", rmdirAgent},   {"moveToTrash", moveToTrashAgent},
    {"touch", touchAgent},   {"mkpath", mkpathAgent}, {"rename", renameAgent}, {"cpfile", cpfileAgent},
    {"cpdir", cpdirAgent},   {"link", linkAgent},     {"unlink", unlinkAgent}};

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 2)  // build in moveToTrash supported path in bin supporte at least in 5.15
auto FileOperation::moveToTrash(const QString& pres, const QString& rels) -> FileOperatorType::RETURN_TYPE {
  const QStringList& paths = pres.split('\n');
  const QStringList& names = rels.split('\n');
  if (paths.size() != names.size()) {
    return {ErrorCode::PATH_NAME_LIST_NOT_EQUAL, {}};
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE revertCmds;
  revertCmds.reserve(pres.size());

  for (int i = 0; i < paths.size(); ++i) {
    const QString& pre = paths[i];
    const QString& rel = names[i];
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      continue;
    }
    QFile file(pth);
    auto ret = file.moveToTrash();
    if (ret) {
      if (file.fileName().isEmpty()) {
        continue;
      }
      revertCmds.append({"rename", "", file.fileName(), "", pth});
    } else {
      return {ErrorCode::UNKNOWN_ERROR, revertCmds};
    }
  }
  return {ErrorCode::OK, revertCmds};
}
#else
auto FileOperation::moveToTrash(const QString& pres, const QString& rels) -> FileOperatorType::RETURN_TYPE {
  // seperated by '\n'
  const QStringList& paths = pres.split('\n');
  const QStringList& names = rels.split('\n');
  if (paths.size() != names.size()) {
    return {ErrorCode::PATH_NAME_LIST_NOT_EQUAL, {}};
  }
  const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmds = RecycleBinHelper::RecycleABatch(paths, names);
  // cmds only contains following 2 commands type:
  //  cmds.append({"mkpath", "", tempLocation});
  //  cmds.append({"rename", path, name, tempLocation, name});
  FileOperatorType::BATCH_COMMAND_LIST_TYPE revertCmds;
  revertCmds.reserve(cmds.size());
  for (const QStringList& cmd : cmds) {
    if (cmd[0] == "rename") {
      if (cmd.size() != 5) {
        return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
      }
      auto ret = QFile::rename(QDir(cmd[1]).absoluteFilePath(cmd[2]), QDir(cmd[3]).absoluteFilePath(cmd[4]));
      if (not ret) {
        return {ErrorCode::UNKNOWN_ERROR, cmds};
      }
      revertCmds.append({"rename", cmd[3], cmd[4], cmd[1], cmd[2]});
    } else if (cmd[0] == "mkpath") {
      if (cmd.size() != 3) {
        return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
      }
      auto ret = QDir(cmd[1]).mkpath(cmd[2]);
      if (not ret) {
        return {ErrorCode::UNKNOWN_ERROR, cmds};
      }
      revertCmds.append({"rmpath", cmd[1], cmd[2]});
    } else {
      qCritical("Invalid command type find in move to trash[%s]", qPrintable(cmd.join(',')));
      return {ErrorCode::COMMAND_TYPE_UNSUPPORT, revertCmds};
    }
  }
  return {ErrorCode::OK, revertCmds};
}
#endif

auto FileOperation::WriteIntoLogFile(const QString& msg) -> bool {
#ifdef _WIN32
  QString logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  QString logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  QFile logFi(QString("%1/%2.log").arg(logPrePath).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
  if (not logFi.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    qCritical("log file[%s] cannot open. ", qPrintable(logFi.fileName()));
    return false;
  }
  QTextStream stream(&logFi);
  stream.setCodec("UTF-8");
  stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\n");
  stream << msg;
  logFi.close();
  return true;
}
