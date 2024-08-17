#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QStringList>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QPair>

#include <QDir>
#include <QMap>
#include <functional>

#include "FileOperatorPub.h"
#include "PublicVariable.h"

class FileOperation {
 public:
  static inline QPair<QString, QString> SplitDirName(const QString& fullPath) {
    auto ind = fullPath.lastIndexOf('/');
    if (ind == -1) {
      return {fullPath, ""};
    }
    if (ind == 0 or fullPath[ind - 1] == ':') {  // in linux "/home", or in Win "C:/Users"
      return {fullPath.left(ind + 1), fullPath.mid(ind + 1)};
    }
    return {fullPath.left(ind), fullPath.mid(ind + 1)};
  }

  static inline auto rmpathAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmpath(pre, rel);
  }

  static inline auto rmpath(const QString& pre, const QString& rel) -> FileOperatorType::RETURN_TYPE {
    // can only remove an empty directory
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QDir(pth).exists()) {
      return {ErrorCode::OK, {}};  // already inexists
    }
    auto ret = QDir(pre).rmpath(rel);
    if (ret) {
      return {ErrorCode::OK, {{"mkpath", pre, rel}}};
    }
    return {ErrorCode::CANNOT_REMOVE_DIR, {}};
  }

  static inline auto mkpathAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::mkpath(pre, rel);
  }

  static inline auto mkpath(const QString& pre, const QString& rel) -> FileOperatorType::RETURN_TYPE {
    QDir preDir(pre);
    if (not preDir.exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    if (preDir.exists(rel)) {
      return {ErrorCode::OK, {}};  // after all it exists
    }

    auto ret = preDir.mkpath(rel);
    if (not ret) {
      return {ErrorCode::UNKNOWN_ERROR, {}};
    }
    return {ErrorCode::OK, {{"rmpath", pre, rel}}};
  }

  static inline auto rmfileAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmfile(pre, rel);
  }

  static auto rmfile(const QString& pre, const QString& rel) -> FileOperatorType::RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFileInfo::exists(pth)) {
      return {ErrorCode::OK, {}};
    }
    auto ret = QDir(pre).remove(rel);
    if (ret) {
      return {ErrorCode::OK, {}};
    } else {
      return {ErrorCode::CANNOT_REMOVE_FILE, {}};
    }
  }

  static inline auto rmdirAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmdir(pre, rel);
  }

  static auto rmdir(const QString& pre, const QString& rel) -> FileOperatorType::RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QDir(pth).exists()) {
      return {ErrorCode::OK, {}};
    }
    auto ret = QDir(pth).removeRecursively();
    if (ret) {
      return {ErrorCode::OK, {}};
    } else {
      return {ErrorCode::CANNOT_REMOVE_DIR, {}};
    }
  }

  static inline auto moveToTrashAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pres = parms[0];  // seperated by '\n'
    const QString& rels = parms[1];
    return FileOperation::moveToTrash(pres, rels);
  }
  static auto moveToTrash(const QString& pres, const QString& rels) -> FileOperatorType::RETURN_TYPE;

  static inline auto renameAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 4) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    const QString& toRel = parms[3];
    return FileOperation::rename(pre, rel, to, toRel);
  }

  static auto rename(const QString& pre, const QString& rel, const QString& to, const QString& toRel) -> FileOperatorType::RETURN_TYPE {
    // a/b -> a/b skip
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    const QString& absNewPath = QDir(to).absoluteFilePath(toRel);

    const bool isOnlyCaseDiffer = rel != toRel and rel.toLower() == toRel.toLower();
    if (QFile(absNewPath).exists() and not isOnlyCaseDiffer) {
      // rename item -> FILE. but there is {file} already. Reject to rename to avoid override {file}
      return {ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST, {}};
    }

    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    const QString& preNewPathFolder = QFileInfo(absNewPath).absolutePath();
    if (not QDir(preNewPathFolder).exists()) {
      auto preNewPathFolderRet = QDir().mkpath(preNewPathFolder);  // only remove dirs
      if (not preNewPathFolderRet) {
        return {ErrorCode::DST_PRE_DIR_CANNOT_MAKE, {}};
      }
      cmds.append({"rmpath", "", preNewPathFolder});
    }
    auto ret = QFile::rename(pth, absNewPath);
    if (not ret) {
      return {ErrorCode::UNKNOWN_ERROR, cmds};
    }
    cmds.append({"rename", to, toRel, pre, rel});
    return {ErrorCode::OK, cmds};
  }
  static inline auto cpfileAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    return FileOperation::cpfile(pre, rel, to);
  }
  static auto cpfile(const QString& pre, const QString& rel, const QString& to) -> FileOperatorType::RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    if (not QDir(to).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    const QString& toPth = QDir(to).absoluteFilePath(rel);
    if (QFile::exists(toPth)) {
      return {ErrorCode::DST_FILE_ALREADY_EXIST, {}};
    }

    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    const QString& prePath = QFileInfo(toPth).absolutePath();
    if (not QDir(prePath).exists()) {
      auto prePathRet = QDir().mkpath(prePath);  // only remove dirs
      if (not prePathRet) {
        return {ErrorCode::DST_PRE_DIR_CANNOT_MAKE, {}};
      }
      cmds.append({"rmpath", "", prePath});
    }
    auto ret = QFile(pth).copy(toPth);
    if (not ret) {
      return {ErrorCode::UNKNOWN_ERROR, cmds};
    }
    cmds.append({"rmfile", to, rel});
    return {ErrorCode::OK, cmds};
  }

  static inline auto cpdirAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    return FileOperation::cpdir(pre, rel, to);
  }

  static auto cpdir(const QString& pre, const QString& rel, const QString& to) -> FileOperatorType::RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    if (not QDir(to).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    const QString& toPth = QDir(to).absoluteFilePath(rel);
    if (QFile::exists(toPth)) {
      return {ErrorCode::DST_FOLDER_ALREADY_EXIST, {}};  // dir or file
    }
    FileOperatorType::BATCH_COMMAND_LIST_TYPE recoverList;
    auto mkRootPthRet = QDir(to).mkpath(rel);
    if (not mkRootPthRet) {
      qDebug("Failed QDir(%s).mkpath(%s)", qPrintable(to), qPrintable(rel));
      return {ErrorCode::UNKNOWN_ERROR, recoverList};
    }
    recoverList.append({"rmpath", to, rel});

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
            return {ErrorCode::DST_FILE_ALREADY_EXIST, recoverList};
          }
        }
        auto mkpthRet = QDir(toPth).mkpath(toRel);
        if (not mkpthRet) {
          qWarning("Failed QDir(%s).mkpath(%s)", qPrintable(toPth), qPrintable(toRel));
          return {ErrorCode::UNKNOWN_ERROR, recoverList};
        }
        recoverList.append({"rmpath", toPth, toRel});
      } else {  // file
        auto cpRet = QFile(fromPth).copy(toPath);
        if (not cpRet) {
          qWarning("Failed QFile(%s).copy(%s)", qPrintable(fromPth), qPrintable(toPath));
          return {ErrorCode::UNKNOWN_ERROR, recoverList};
        }
        recoverList.append({"rmfile", toPth, toRel});
      }
    }
    return {ErrorCode::OK, recoverList};
  }

  static inline auto touchAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::touch(pre, rel);
  }

  static auto touch(const QString& pre, const QString& rel) -> FileOperatorType::RETURN_TYPE {
    if (not QDir(pre).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    QFile textFile(pth);
    if (textFile.exists()) {
      return {ErrorCode::OK, {}};  // after all it exists
    }
    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    const QString& prePath = QFileInfo(pth).absolutePath();
    if (not QDir(prePath).exists()) {
      auto prePathRet = QDir().mkpath(prePath);
      if (not prePathRet) {
        return {ErrorCode::DST_PRE_DIR_CANNOT_MAKE, cmds};
      }
      cmds.append({"rmpath", "", prePath});
    }
    auto ret = textFile.open(QIODevice::NewOnly);
    if (not ret) {
      return {ErrorCode::UNKNOWN_ERROR, cmds};
    }
    cmds.append({"rmfile", pre, rel});
    return {ErrorCode::OK, cmds};
  }

  static auto WriteIntoLogFile(const QString& msg) -> bool;

  static auto executer(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& aBatch, FileOperatorType::BATCH_COMMAND_LIST_TYPE& srcCommand)-> FileOperatorType::EXECUTE_RETURN_TYPE;

  static inline auto linkAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2 and parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    if (parms.size() == 2) {
      return FileOperation::link(pre, rel);
    }
    const QString& to = parms[2];
    return FileOperation::link(pre, rel, to);
  }

  static inline auto link(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath) -> FileOperatorType::RETURN_TYPE {
    const QString pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    if (not QDir(to).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    QString toPath(QDir(to).absoluteFilePath(rel) + ".lnk");
    QFile toFile(toPath);

    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    if (toFile.exists()) {
      if (not QFile(toPath).moveToTrash()) {
        return {ErrorCode::CANNOT_REMOVE_FILE, cmds};
      }
      cmds.append({"rename", "", toFile.fileName(), "", toPath});
    }

    QString prePath(QFileInfo(toPath).absolutePath());
    if (not QDir(prePath).exists()) {
      const auto prePathRet = QDir().mkpath(prePath);
      if (not prePathRet) {
        return {ErrorCode::DST_PRE_DIR_CANNOT_MAKE, cmds};
      }
      cmds.append({"rmpath", "", prePath});
    }
    if (not QFile::link(pth, toPath)) {
      return {ErrorCode::CANNOT_MAKE_LINK, cmds};
    }

    cmds.append({"unlink", pre, rel + ".lnk", to});
    return {ErrorCode::OK, cmds};
  }

  static inline auto unlinkAgent(const QStringList& parms) -> FileOperatorType::RETURN_TYPE {
    if (parms.size() != 2 and parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    if (parms.size() == 2) {
      return FileOperation::unlink(pre, rel);
    }
    const QString& to = parms[2];
    return FileOperation::unlink(pre, rel, to);
  }

  static inline auto unlink(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath) -> FileOperatorType::RETURN_TYPE {
    FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
    QString toPath(QDir(to).absoluteFilePath(rel));
    if (not QFile::exists(toPath)) {
      return {ErrorCode::OK, cmds};  // after all it not exist
    }

    const auto ret = QDir().remove(toPath);
    if (not ret) {
      return {ErrorCode::CANNOT_REMOVE_LINK, cmds};
    }
    cmds.append({"link", pre, rel.left(rel.size() - 4), to});  // move the trailing ".lnk"
    return {ErrorCode::OK, cmds};
  }

  /*    LambdaTable: dict[
      str, Callable[[], tuple[ErrorCode, list[tuple]]]] = \
      {"rmfile": rmfile, "rmpath": rmpath, "rmdir": rmdir, "moveToTrash": moveToTrash,
       "touch": touch, "mkpath": mkpath,
       "rename": rename,
       "cpfile": cpfile, "cpdir": cpdir,
       "link": link, "unlink": unlink}*/
  static const QMap<QString, std::function<FileOperatorType::RETURN_TYPE(const QStringList& QStringList)>> LambdaTable;
};

#endif  // FILEOPERATION_H
