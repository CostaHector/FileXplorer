#include "FileOperation.h"
#include "public/PathTool.h"

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

RETURN_TYPE executer(const BATCH_COMMAND_LIST_TYPE& aBatch, BATCH_COMMAND_LIST_TYPE& srcCommand) {
  typedef std::function<RETURN_TYPE(const QStringList&)> FILE_OPERATION_FUNC;
  static const FILE_OPERATION_FUNC LAMBDA_TABLE[FILE_OPERATOR_E::OPERATOR_BUTT]{
#define FILE_OPERATOR_KEY_ITEM(enu, val, func, FACTORY_PARAMS, FACTORY_ARGVS) func,
      FILE_OPERATOR_TYPE_FIELD_MAPPING
#undef FILE_OPERATOR_KEY_ITEM
  };

  BATCH_COMMAND_LIST_TYPE recoverList;
  for (int i = 0; i < aBatch.size(); ++i) {
    const ACMD& cmds = aBatch[i];
    if (cmds.isEmpty() || cmds.op >= FILE_OPERATOR_E::OPERATOR_BUTT) {
      continue;
    }

    const RETURN_TYPE returnEle = LAMBDA_TABLE[cmds.op](cmds.lst);
    if (!returnEle) {
      qCritical("[Error]%s", qPrintable(cmds.toStr(returnEle.ret)));
      return RETURN_TYPE{returnEle.ret, QList<ACMD>(recoverList.crbegin(), recoverList.crend())};
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
  // in-place reverse
  return RETURN_TYPE{ErrorCode::OK, QList<ACMD>(recoverList.crbegin(), recoverList.crend())};
}

RETURN_TYPE moveToTrash(const QString& pre, const QString& rel) {
  BATCH_COMMAND_LIST_TYPE revertCmds;
  if (pre.isEmpty() && rel.isEmpty()) {
    return {OK, revertCmds};
  }
  const QString& pth = PathTool::Path2Join(pre, rel);
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
    revertCmds.append(ACMD::GetInstRENAME("", file.fileName(), pth));
  } else {
    return {UNKNOWN_ERROR, revertCmds};
  }
  return {OK, revertCmds};
}

RETURN_TYPE rmFolderForceAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& dirName = parms[1];
  return FileOperation::rmFolderForce(pre, dirName);
}

RETURN_TYPE rmFolderForce(const QString& pre, const QString& dirName) {
  const QString& pth = PathTool::Path2Join(pre, dirName);
  if (!QFile::exists(pth)) {
    return {};  // already inexists
  }
  if (!QDir{pth}.removeRecursively()) {
    return {CANNOT_REMOVE_FOLDER_FORCE, {}};
  }
  return {};
}

RETURN_TYPE rmpathAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& dirPath = parms[1];
  return FileOperation::rmpath(pre, dirPath);
}

RETURN_TYPE rmpath(const QString& pre, const QString& dirPath) {
  const QString& pth = PathTool::Path2Join(pre, dirPath);
  if (!QFile::exists(pth)) {
    return {};  // already inexists
  }
  BATCH_COMMAND_LIST_TYPE revertCmds;
  QDir realDir{pre};
  if (realDir.exists(dirPath)) {
    if (!realDir.rmdir(dirPath)) {
      return {CANNOT_REMOVE_DIR, revertCmds};
    }
    revertCmds.append(ACMD::GetInstMKDIR(pre, dirPath));
  }
  int lastSlashIndex = dirPath.lastIndexOf('/');
  while (lastSlashIndex != -1) {
    const QString& currentFolder = dirPath.left(lastSlashIndex);
    if (realDir.exists(currentFolder)) {
      if (!realDir.rmdir(currentFolder)) {
        return {CANNOT_REMOVE_DIR, revertCmds};
      }
      revertCmds.append(ACMD::GetInstMKDIR(pre, currentFolder));
    }
    lastSlashIndex = dirPath.lastIndexOf('/', lastSlashIndex - 1);
  }
  return {OK, revertCmds};
  // in built-in rmpath
  // last level of dirPath is empty, say "home/to/path" contains nothing, removed
  // second to last level of dirPath is empty, say "home/to", contains nothing, removed
  // first level of dirPath is empty, say "home", contains nothing, removed

  // rel and subfolder under rel will or be removed
}

RETURN_TYPE mkpathAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::mkpath(pre, rel);
}

ErrorCode mkpathCore(const QString& pre, const QString& dirPath, BATCH_COMMAND_LIST_TYPE& revertCmds) {
  QDir preDir{pre};
  int firstIndexOfSlash = dirPath.indexOf('/');
  while (firstIndexOfSlash != -1) {
    const QString& currentFolder = dirPath.left(firstIndexOfSlash);
    if (!preDir.exists(currentFolder)) {
      if (!preDir.mkdir(currentFolder)) {
        return CANNOT_MAKE_DIR;
      }
      revertCmds.append(ACMD::GetInstRMDIR(pre, currentFolder));
    }
    firstIndexOfSlash = dirPath.indexOf('/', firstIndexOfSlash + 1);
  }
  if (!preDir.exists(dirPath)) {
    if (!preDir.mkdir(dirPath)) {
      return CANNOT_MAKE_DIR;
    }
    revertCmds.append(ACMD::GetInstRMDIR(pre, dirPath));
  }
  return OK;
}

RETURN_TYPE mkpath(const QString& pre, const QString& dirPath) {
  QDir preDir{pre};
  if (!preDir.exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  if (preDir.exists(dirPath)) {
    return {};  // already exists
  }
  BATCH_COMMAND_LIST_TYPE revertCmds;
  ErrorCode errorCode = mkpathCore(pre, dirPath, revertCmds);
  return {errorCode, revertCmds};
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
  const QString& pth = PathTool::Path2Join(pre, rel);
  if (!QFile::exists(pth)) {
    return {};
  }
  if (!QDir{pre}.remove(rel)) {
    return {CANNOT_REMOVE_FILE, {}};
  }
  return {};
}

RETURN_TYPE mkdirAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& dirName = parms[1];
  return FileOperation::mkdir(pre, dirName);
}

RETURN_TYPE mkdir(const QString& pre, const QString& dirName) {
  const QString& pth = PathTool::Path2Join(pre, dirName);
  if (QDir{pth}.exists()) {
    return {};
  }
  if (!QDir{pre}.mkdir(dirName)) {
    return {CANNOT_REMOVE_DIR, {}};
  }
  // empty under "pre/rel"
  // only remove last level of "pre/rel", i.e., rel
  return {OK, {ACMD::GetInstRMDIR(pre, dirName)}};
}

RETURN_TYPE rmdirAgent(const QStringList& parms) {
  if (parms.size() != 2) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& pre = parms[0];
  const QString& rel = parms[1];
  return FileOperation::rmdir(pre, rel);
}

RETURN_TYPE rmdir(const QString& pre, const QString& dirName) {
  const QString& pth = PathTool::Path2Join(pre, dirName);
  if (!QDir{pth}.exists()) {
    return {};
  }
  if (!QDir{pre}.rmdir(dirName)) {
    return {CANNOT_REMOVE_DIR, {}};
  }
  // empty under "pre/rel"
  // only remove last level of "pre/rel", i.e., rel
  return {OK, {ACMD::GetInstMKDIR(pre, dirName)}};
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
  if (parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& srcPath = parms[0];
  const QString& oldCompleteName = parms[1];
  const QString& newCompleteName = parms[2];
  return FileOperation::rename(srcPath, oldCompleteName, newCompleteName);
}

RETURN_TYPE rename(const QString& srcPath, const QString& oldCompleteName, const QString& newCompleteName) {
  if (oldCompleteName == newCompleteName) {
    // name unchange, no need continue. return right now
    return {};
  }
  const QString& absOldPath = PathTool::Path2Join(srcPath, oldCompleteName);
  if (!QFile::exists(absOldPath)) {
    // source item not exist at all. return right now
    return {SRC_INEXIST, {}};
  }
  const QString& absNewPath = PathTool::Path2Join(srcPath, newCompleteName);
  // name now differ in case/char/char count

  if (oldCompleteName.toLower() == newCompleteName.toLower()) {
    // only differ in case
#ifndef _WIN32
    // Linux system allow two item name only differ in case. QFile::rename("a.txt", "A.txt") will cause file overwrite
    if (QFile::exists(absNewPath)) {
      return {DST_FILE_OR_PATH_ALREADY_EXIST, {}};
    }
#endif
    // Windows system may prevent two items(Only differ in case) place/create/moved in one folder. so newCompleteName will not exist under srcPath
    if (!QFile::rename(absOldPath, absNewPath)) {
      return {CANNOT_RENAME, {}};  // file being occupied by others or permission
    }
    return {OK, {ACMD::GetInstRENAME(srcPath, newCompleteName, oldCompleteName)}};
  }

  // name now differ in char/char count
  if (QFile::exists(absNewPath)) {  // windows QFile::exists is case-insensitive, linux case-sensitive matter
    return {DST_FILE_OR_PATH_ALREADY_EXIST, {}};
  }

  if (!QFile::rename(absOldPath, absNewPath)) {
    return {CANNOT_RENAME, {}};  // file being occupied by others or permission
  }
  return {OK, {ACMD::GetInstRENAME(srcPath, newCompleteName, oldCompleteName)}};
}

RETURN_TYPE mvAgent(const QStringList& parms) {
  if (parms.size() != 3) {
    return {OPERATION_PARMS_NOT_MATCH, {}};
  }
  const QString& srcPath = parms[0];
  const QString& relToItem = parms[1];
  const QString& dstPath = parms[2];
  return FileOperation::mv(srcPath, relToItem, dstPath);
}

RETURN_TYPE mv(const QString& srcPath, const QString& relToItem, const QString& dstPath) {
  if (srcPath.toLower() == dstPath.toLower()) {  // folder belong dstPath no change
    return {};
  }
  const QString& absOldPath = PathTool::Path2Join(srcPath, relToItem);
  if (!QFile::exists(absOldPath)) {
    return {SRC_INEXIST, {}};
  }
  const QString& absNewPath = PathTool::Path2Join(dstPath, relToItem);
  if (QFile::exists(absNewPath)) {  // already exist, ignore file or folder
    return {DST_FILE_OR_PATH_ALREADY_EXIST, {}};
  }

  BATCH_COMMAND_LIST_TYPE revertCmds;
  const QString& preNewPathFolder = PathTool::absolutePath(absNewPath);
  if (QFile::exists(preNewPathFolder)) {
    if (!QFileInfo{preNewPathFolder}.isDir()) {
      return {DST_PRE_DIR_OCCUPIED_BY_FILE, {}};
    }
  } else {
    const QString relativeToItem{preNewPathFolder.mid(dstPath.size() + 1)};
    ErrorCode errorCode = mkpathCore(dstPath, relativeToItem, revertCmds);
    if (errorCode != OK) {
      return {errorCode, revertCmds};
    }
  }
  if (!QFile::rename(absOldPath, absNewPath)) {
    return {CANNOT_MV, revertCmds};
  }

  revertCmds.append(ACMD::GetInstMV(dstPath, relToItem, srcPath));
  return {OK, revertCmds};
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
  const QString& pth = PathTool::Path2Join(pre, rel);
  if (!QFile::exists(pth)) {
    return {SRC_INEXIST, {}};
  }
  if (!QDir{to}.exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  const QString& toPth = PathTool::Path2Join(to, rel);
  if (QFile::exists(toPth)) {
    return {DST_FILE_ALREADY_EXIST, {}};
  }

  BATCH_COMMAND_LIST_TYPE cmds;
  const QString& prePath = PathTool::absolutePath(toPth);
  if (!QDir{prePath}.exists()) {
    const QString relToPath{prePath.mid(to.size() + 1)};
    ErrorCode errorCode = mkpathCore(to, relToPath, cmds);
    if (errorCode != OK) {
      return {errorCode, cmds};
    }
  }
  auto ret = QFile{pth}.copy(toPth);
  if (!ret) {
    return {UNKNOWN_ERROR, cmds};
  }
  cmds.append(ACMD::GetInstRMFILE(to, rel));
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
  if (!QFileInfo{pre, rel}.isDir()) {
    return {SRC_INEXIST, {}};
  }
  if (!QDir{to}.exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  if (QFileInfo{to, rel}.isDir()) {
    return {DST_FOLDER_ALREADY_EXIST, {}};  // dir or file
  }
  BATCH_COMMAND_LIST_TYPE recoverList;
  if (!QDir{to}.mkpath(rel)) {
    qWarning("Failed QDir(%s).mkpath(%s)", qPrintable(to), qPrintable(rel));
    return {CANNOT_MAKE_PATH, recoverList};
  }
  recoverList.append(ACMD::GetInstRMPATH(to, rel));

  const QString& pth = PathTool::Path2Join(pre, rel);
  const QString& toPth = PathTool::Path2Join(to, rel);
  // or shutil.copytree(pth, toPth)
  QDirIterator src{pth, {}, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDirIterator::Subdirectories};
  int relN = pth.size() + 1;
  while (src.hasNext()) {
    src.next();
    const QString& fromPth = src.filePath();
    const QString& toRel = fromPth.mid(relN);
    const QString& toPath = PathTool::Path2Join(toPth, toRel);
    const QFileInfo& srcFile{src.fileInfo()};
    const QFileInfo dstFile{toPath};
    if (dstFile.exists()) {
      if (dstFile.isDir() && srcFile.isDir()) {
        continue;
      }
      qWarning("conflict two item:\nsrc: %lld Byte(s)[%s]\ndst: %lld Byte(s)[%s]",  //
               srcFile.size(), qPrintable(fromPth),                                 //
               dstFile.size(), qPrintable(toPath));
      return {DST_FILE_ALREADY_EXIST, recoverList};
    }
    if (srcFile.isDir()) {  // dir
      // dir not exist, create it
      if (!QDir(toPth).mkpath(toRel)) {
        qWarning("Failed QDir(%s).mkpath(%s)", qPrintable(toPth), qPrintable(toRel));
        return {UNKNOWN_ERROR, recoverList};
      }
      recoverList.append(ACMD::GetInstRMPATH(toPth, toRel));
    } else {  // file
      if (!QFile(fromPth).copy(toPath)) {
        qWarning("Failed QFile(%s).copy(%s)", qPrintable(fromPth), qPrintable(toPath));
        return {UNKNOWN_ERROR, recoverList};
      }
      recoverList.append(ACMD::GetInstRMFILE(toPth, toRel));
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
  const QString& pth = PathTool::Path2Join(pre, rel);
  QFile textFile(pth);
  if (textFile.exists()) {
    return {};  // after all it exists
  }
  BATCH_COMMAND_LIST_TYPE revertCmds;
  const QString& prePath = PathTool::absolutePath(pth);
  if (!QDir{prePath}.exists()) {
    const QString relativeToItem{prePath.mid(pre.size() + 1)};
    ErrorCode errorCode = mkpathCore(pre, relativeToItem, revertCmds);
    if (errorCode != OK) {
      return {errorCode, revertCmds};
    }
  }
  if (!textFile.open(QIODevice::NewOnly)) {
    return {UNKNOWN_ERROR, revertCmds};
  }
  textFile.close();
  revertCmds.append(ACMD::GetInstRMFILE(pre, rel));
  return {OK, revertCmds};
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
  const QString pth = PathTool::Path2Join(pre, rel);
  if (!QFile::exists(pth)) {
    return {SRC_INEXIST, {}};
  }
  if (!QDir{to}.exists()) {
    return {DST_DIR_INEXIST, {}};
  }
  QString toPath{PathTool::Path2Join(to, rel) + ".lnk"};

  BATCH_COMMAND_LIST_TYPE cmds;
  QFile toFile{toPath};
  if (toFile.exists()) {
    if (!QFile{toPath}.moveToTrash()) {
      return {CANNOT_REMOVE_FILE, cmds};
    }
    cmds.append(ACMD::GetInstRENAME("", toFile.fileName(), toPath));
  }

  QString prePath{PathTool::absolutePath(toPath)};
  if (!QDir{prePath}.exists()) {
    const auto prePathRet = QDir{}.mkpath(prePath);
    if (!prePathRet) {
      return {DST_PRE_DIR_CANNOT_MAKE, cmds};
    }
    cmds.append(ACMD::GetInstRMPATH("", prePath));
  }
  if (!QFile::link(pth, toPath)) {
    return {CANNOT_MAKE_LINK, cmds};
  }

  cmds.append(ACMD::GetInstUNLINK(pre, rel + ".lnk", to));
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
  BATCH_COMMAND_LIST_TYPE cmds;
  QString toPath(PathTool::Path2Join(to, rel));
  if (!QFile::exists(toPath)) {
    return {OK, cmds};  // after all it not exist
  }

  if (!QDir().remove(toPath)) {
    return {CANNOT_REMOVE_LINK, cmds};
  }
  cmds.append(ACMD::GetInstLINK(pre, rel.left(rel.size() - 4), to));  // move the trailing ".lnk"
  return {OK, cmds};
}

}  // namespace FileOperation
