#include "FileOperation.h"

#include <QDateTime>
#include <QStringList>

#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QFileInfo>

#include <QPair>
#include <QMap>
#include <functional>

namespace FileOperation {

using namespace FileOperatorType;

RETURN_TYPE moveToTrash(const QString& pre, const QString& rel) {
  BATCH_COMMAND_LIST_TYPE revertCmds;
  if (pre.isEmpty() && rel.isEmpty()) {
    return {OK, revertCmds};
  }
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QFile::exists(pth)) {
    return {OK, revertCmds};
  }
  QFile file(pth);
  auto ret = file.moveToTrash();
  if (ret) {
    if (file.fileName().isEmpty()) {
      qCritical("The file[%s] name in trashbin is not accessible", qPrintable(pth));
      return {UNKNOWN_ERROR, revertCmds};
    }
    revertCmds.append(ACMD{RENAME, {"", file.fileName(), "", pth}});
  } else {
    return {UNKNOWN_ERROR, revertCmds};
  }
  return {OK, revertCmds};
}

RETURN_TYPE executer(const BATCH_COMMAND_LIST_TYPE& aBatch, BATCH_COMMAND_LIST_TYPE& srcCommand) {
  static const QMap<FileOperator, std::function<RETURN_TYPE(const QStringList&)>> LAMBDA_TABLE = {{RMFILE, rmfileAgent}, {RMPATH, rmpathAgent}, {RMDIR, rmdirAgent},   {MOVETOTRASH, moveToTrashAgent},
                                                                                                  {TOUCH, touchAgent},   {MKPATH, mkpathAgent}, {RENAME, renameAgent}, {CPFILE, cpfileAgent},
                                                                                                  {CPDIR, cpdirAgent},   {LINK, linkAgent},     {UNLINK, unlinkAgent}};
  BATCH_COMMAND_LIST_TYPE recoverList;
  QStringList failCmd;

  for (int i = 0; i < aBatch.size(); ++i) {
    const ACMD& cmds = aBatch[i];
    if (cmds.isEmpty()) {
      continue;
    }

    const RETURN_TYPE returnEle = LAMBDA_TABLE[cmds.op](cmds.lst);
    if (!returnEle) {
      failCmd << cmds.toStr();
    }

    if (cmds.op == MOVETOTRASH && !srcCommand.isEmpty()) {  // name in trashbin is now changed compared with last time in trashbin
      if (returnEle.size() > 1) {
        qCritical("moveToTrash recover commands count[%d] can only <= 1", returnEle.size());
        return {UNKNOWN_ERROR, {}};
      } else if (returnEle.size() == 1) {
        srcCommand.rbegin()[i] = returnEle[0];
      } else {
        srcCommand.rbegin()[i].clear();
      }
    }
    recoverList += returnEle;
  }

  ErrorCode errorCode = ErrorCode::OK;
  if (failCmd.isEmpty()) {
    errorCode = ErrorCode::UNKNOWN_ERROR;
    qCritical("Below %d command(s) failed:\n%s", failCmd.size(), qPrintable(failCmd.join('\n')));
  }
  // in-place reverse
  return RETURN_TYPE{errorCode, QList<ACMD>(recoverList.crbegin(), recoverList.crend())};
}

RETURN_TYPE rmpathAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::rmpath(pre, rel);
}

RETURN_TYPE rmpath(const QString& pre, const QString& rel) {
  // can only remove an empty directory
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QDir(pth).exists()) {
    return {OK, {}};  // already inexists
  }
  auto ret = QDir(pre).rmpath(rel);
  if (ret) {
    return {OK, {FileOperatorType::ACMD{FileOperatorType::MKPATH, {pre, rel}}}};
  }
  return {CANNOT_REMOVE_DIR, {}};
}

RETURN_TYPE mkpathAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::mkpath(pre, rel);
}

RETURN_TYPE mkpath(const QString& pre, const QString& rel) {
  QDir preDir(pre);
  if (not preDir.exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  if (preDir.exists(rel)) {
    return {OK, {}};  // after all it exists
  }

  auto ret = preDir.mkpath(rel);
  if (not ret) {
    return {UNKNOWN_ERROR, {}};
  }
  return {OK, {FileOperatorType::ACMD{FileOperatorType::RMPATH, {pre, rel}}}};
}

RETURN_TYPE rmfileAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::rmfile(pre, rel);
}

RETURN_TYPE rmfile(const QString& pre, const QString& rel) {
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QFileInfo::exists(pth)) {
    return {OK, {}};
  }
  auto ret = QDir(pre).remove(rel);
  if (ret) {
    return {OK, {}};
  } else {
    return {CANNOT_REMOVE_FILE, {}};
  }
}

RETURN_TYPE rmdirAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::rmdir(pre, rel);
}

RETURN_TYPE rmdir(const QString& pre, const QString& rel) {
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QDir(pth).exists()) {
    return {OK, {}};
  }
  auto ret = QDir(pth).removeRecursively();
  if (ret) {
    return {OK, {}};
  } else {
    return {CANNOT_REMOVE_DIR, {}};
  }
}

RETURN_TYPE moveToTrashAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pres = parms[0];  // seperated by '\n'
  const QString& rels = parms[1];
  return FileOperation::moveToTrash(pres, rels);
}

RETURN_TYPE renameAgent(const QStringList& parms) {
  if (parms.size() != 4) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  const QString& to = parms[2];
  const QString& toRel = parms[3];
  return FileOperation::rename(pre, rel, to, toRel);
}

RETURN_TYPE rename(const QString& pre, const QString& rel, const QString& to, const QString& toRel) {
  // a/b -> a/b skip
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QFile::exists(pth)) {
    return {SRC_INEXIST, {}};
  }
  const QString& absNewPath = QDir(to).absoluteFilePath(toRel);

  const bool isOnlyCaseDiffer = rel != toRel and rel.toLower() == toRel.toLower();
  if (QFile(absNewPath).exists() and not isOnlyCaseDiffer) {
    // rename item -> FILE. but there is {file} already. Reject to rename to avoid override {file}
    return {DST_FILE_OR_PATH_ALREADY_EXIST, {}};
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  const QString& preNewPathFolder = QFileInfo(absNewPath).absolutePath();
  if (not QDir(preNewPathFolder).exists()) {
    auto preNewPathFolderRet = QDir().mkpath(preNewPathFolder);  // only remove dirs
    if (not preNewPathFolderRet) {
      return {DST_PRE_DIR_CANNOT_MAKE, {}};
    }
    cmds.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {"", preNewPathFolder}});
  }
  auto ret = QFile::rename(pth, absNewPath);
  if (not ret) {
    return {UNKNOWN_ERROR, cmds};
  }
  cmds.append(FileOperatorType::ACMD{FileOperatorType::RENAME, {to, toRel, pre, rel}});
  return {OK, cmds};
}
RETURN_TYPE cpfileAgent(const QStringList& parms) {
  if (parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  const QString& to = parms[2];
  return FileOperation::cpfile(pre, rel, to);
}
RETURN_TYPE cpfile(const QString& pre, const QString& rel, const QString& to) {
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  if (not QFile::exists(pth)) {
    return {SRC_INEXIST, {}};
  }
  if (not QDir(to).exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  const QString& toPth = QDir(to).absoluteFilePath(rel);
  if (QFile::exists(toPth)) {
    return {DST_FILE_ALREADY_EXIST, {}};
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  const QString& prePath = QFileInfo(toPth).absolutePath();
  if (not QDir(prePath).exists()) {
    auto prePathRet = QDir().mkpath(prePath);  // only remove dirs
    if (not prePathRet) {
      return {DST_PRE_DIR_CANNOT_MAKE, {}};
    }
    cmds.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {"", prePath}});
  }
  auto ret = QFile(pth).copy(toPth);
  if (not ret) {
    return {UNKNOWN_ERROR, cmds};
  }
  cmds.append(FileOperatorType::ACMD{FileOperatorType::RMFILE, {to, rel}});
  return {OK, cmds};
}

RETURN_TYPE cpdirAgent(const QStringList& parms) {
  if (parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  const QString& to = parms[2];
  return FileOperation::cpdir(pre, rel, to);
}

RETURN_TYPE cpdir(const QString& pre, const QString& rel, const QString& to) {
  if (!QFileInfo(pre, rel).isDir()) {
    return {SRC_INEXIST, {}};
  }
  if (!QDir(to).exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  if (QFileInfo(to, rel).isDir()) {
    return {DST_FOLDER_ALREADY_EXIST, {}};  // dir or file
  }
  FileOperatorType::BATCH_COMMAND_LIST_TYPE recoverList;
  if (!QDir(to).mkpath(rel)) {
    qDebug("Failed QDir(%s).mkpath(%s)", qPrintable(to), qPrintable(rel));
    return {UNKNOWN_ERROR, recoverList};
  }
  recoverList.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {to, rel}});

  const QString& pth = QDir(pre).absoluteFilePath(rel);
  const QString& toPth = QDir(to).absoluteFilePath(rel);
  // or shutil.copytree(pth, toPth)
  QDirIterator src(pth, {}, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDirIterator::Subdirectories);
  int relN = pth.size() + 1;
  while (src.hasNext()) {
    src.next();
    const QString& fromPth = src.filePath();
    const QString& toRel = fromPth.mid(relN);
    const QString& toPath = QDir(toPth).absoluteFilePath(toRel);
    if (src.fileInfo().isDir()) {  // dir
      if (QFile::exists(toPath)) {
        if (QFileInfo(toPath).isFile()) {
          return {DST_FILE_ALREADY_EXIST, recoverList};
        }
      }
      auto mkpthRet = QDir(toPth).mkpath(toRel);
      if (not mkpthRet) {
        qWarning("Failed QDir(%s).mkpath(%s)", qPrintable(toPth), qPrintable(toRel));
        return {UNKNOWN_ERROR, recoverList};
      }
      recoverList.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {toPth, toRel}});
    } else {  // file
      auto cpRet = QFile(fromPth).copy(toPath);
      if (not cpRet) {
        qWarning("Failed QFile(%s).copy(%s)", qPrintable(fromPth), qPrintable(toPath));
        return {UNKNOWN_ERROR, recoverList};
      }
      recoverList.append(FileOperatorType::ACMD{FileOperatorType::RMFILE, {toPth, toRel}});
    }
  }
  return {OK, recoverList};
}

RETURN_TYPE touchAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::touch(pre, rel);
}

RETURN_TYPE touch(const QString& pre, const QString& rel) {
  if (not QDir(pre).exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  const QString& pth = QDir(pre).absoluteFilePath(rel);
  QFile textFile(pth);
  if (textFile.exists()) {
    return {OK, {}};  // after all it exists
  }
  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  const QString& prePath = QFileInfo(pth).absolutePath();
  if (not QDir(prePath).exists()) {
    auto prePathRet = QDir().mkpath(prePath);
    if (not prePathRet) {
      return {DST_PRE_DIR_CANNOT_MAKE, cmds};
    }
    cmds.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {"", prePath}});
  }
  auto ret = textFile.open(QIODevice::NewOnly);
  if (not ret) {
    return {UNKNOWN_ERROR, cmds};
  }
  cmds.append(FileOperatorType::ACMD{FileOperatorType::RMFILE, {pre, rel}});
  return {OK, cmds};
}

RETURN_TYPE linkAgent(const QStringList& parms) {
  if (parms.size() != 2 and parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  if (parms.size() == 2) {
    return FileOperation::link(pre, rel);
  }
  const QString& to = parms[2];
  return FileOperation::link(pre, rel, to);
}

RETURN_TYPE link(const QString& pre, const QString& rel, const QString& to) {
  const QString pth = QDir(pre).absoluteFilePath(rel);
  if (not QFile::exists(pth)) {
    return {SRC_INEXIST, {}};
  }
  if (not QDir(to).exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  QString toPath(QDir(to).absoluteFilePath(rel) + ".lnk");
  QFile toFile(toPath);

  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  if (toFile.exists()) {
    if (not QFile(toPath).moveToTrash()) {
      return {CANNOT_REMOVE_FILE, cmds};
    }
    cmds.append(FileOperatorType::ACMD{FileOperatorType::RENAME, {"", toFile.fileName(), "", toPath}});
  }

  QString prePath(QFileInfo(toPath).absolutePath());
  if (not QDir(prePath).exists()) {
    const auto prePathRet = QDir().mkpath(prePath);
    if (not prePathRet) {
      return {DST_PRE_DIR_CANNOT_MAKE, cmds};
    }
    cmds.append(FileOperatorType::ACMD{FileOperatorType::RMPATH, {"", prePath}});
  }
  if (not QFile::link(pth, toPath)) {
    return {CANNOT_MAKE_LINK, cmds};
  }

  cmds.append(FileOperatorType::ACMD{FileOperatorType::UNLINK, {pre, rel + ".lnk", to}});
  return {OK, cmds};
}

RETURN_TYPE unlinkAgent(const QStringList& parms) {
  if (parms.size() != 2 and parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  if (parms.size() == 2) {
    return FileOperation::unlink(pre, rel);
  }
  const QString& to = parms[2];
  return FileOperation::unlink(pre, rel, to);
}

RETURN_TYPE unlink(const QString& pre, const QString& rel, const QString& to) {
  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  QString toPath(QDir(to).absoluteFilePath(rel));
  if (not QFile::exists(toPath)) {
    return {OK, cmds};  // after all it not exist
  }

  const auto ret = QDir().remove(toPath);
  if (not ret) {
    return {CANNOT_REMOVE_LINK, cmds};
  }
  cmds.append(FileOperatorType::ACMD{FileOperatorType::LINK, {pre, rel.left(rel.size() - 4), to}});  // move the trailing ".lnk"
  return {OK, cmds};
}

}  // namespace FileOperation
