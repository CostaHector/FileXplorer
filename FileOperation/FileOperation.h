#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QStringList>

namespace ErrorCode {
constexpr int OK = 0;
constexpr int SRC_PRE_DIR_INEXIST = 1;
constexpr int SRC_FILE_INEXIST = 2;
constexpr int SRC_DIR_INEXIST = 3;
constexpr int SRC_INEXIST = 4;
constexpr int DST_DIR_INEXIST = 5;
constexpr int DST_PRE_DIR_CANNOT_MAKE = 6;
constexpr int DST_FOLDER_ALREADY_EXIST = 7;
constexpr int DST_FILE_ALREADY_EXIST = 8;
constexpr int DST_FILE_OR_PATH_ALREADY_EXIST = 9;
constexpr int CANNOT_REMOVE_FILE = 10;
constexpr int CANNOT_REMOVE_DIR = 11;
constexpr int CANNOT_MAKE_LINK = 12;
constexpr int DST_LINK_INEXIST = 13;
constexpr int CANNOT_REMOVE_LINK = 14;
constexpr int OPERATION_NOT_AVAILABLE = 15;
constexpr int OPERATION_PARMS_NOT_MATCH = 16;
constexpr int UNKNOWN_ERROR = -1;
};  // namespace ErrorCode

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QPair>

#include <QDir>
#include <QMap>
#include <functional>
#include "PublicVariable.h"

class FileOperation {
 public:
  using BATCH_COMMAND_LIST_TYPE = QList<QStringList>;
  using RETURN_TYPE = QPair<int, BATCH_COMMAND_LIST_TYPE>;
  using EXECUTE_RETURN_TYPE = QPair<bool, BATCH_COMMAND_LIST_TYPE>;

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

  static inline auto rmpathAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmpath(pre, rel);
  }

  static inline auto rmpath(const QString& pre, const QString& rel) -> RETURN_TYPE {
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

  static inline auto mkpathAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::mkpath(pre, rel);
  }

  static inline auto mkpath(const QString& pre, const QString& rel) -> RETURN_TYPE {
    QDir preDir(pre);
    if (not preDir.exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    if (preDir.exists(rel)) {
      return {ErrorCode::OK, {}};  // after all it exists
    }

    auto ret = preDir.mkpath(rel);
    if (ret) {
      return {ErrorCode::OK, {{"rmpath", pre, rel}}};
    } else {
      return {ErrorCode::UNKNOWN_ERROR, {}};
    }
  }

  static inline auto rmfileAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmfile(pre, rel);
  }

  static auto rmfile(const QString& pre, const QString& rel) -> RETURN_TYPE {
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

  static inline auto rmdirAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::rmdir(pre, rel);
  }

  static auto rmdir(const QString& pre, const QString& rel) -> RETURN_TYPE {
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

  static inline auto moveToTrashAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::moveToTrash(pre, rel);
  }

  static inline auto moveToTrash(const QString& pre, const QString& rel) -> RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::OK, {}};
    }
    QFile file(pth);
    auto ret = file.moveToTrash();
    if (ret) {
      return {ErrorCode::OK, {{"rename", "", file.fileName(), "", pth}}};
    } else {
      return {ErrorCode::UNKNOWN_ERROR, {}};
    }
  }

  static inline auto renameAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 4) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    const QString& toRel = parms[3];
    return FileOperation::rename(pre, rel, to, toRel);
  }

  static auto rename(const QString& pre, const QString& rel, const QString& to, const QString& toRel) -> RETURN_TYPE {
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    const QString& absNewPath = QDir(to).absoluteFilePath(toRel);
    if (QFile(absNewPath).exists()) {
      return {ErrorCode::DST_FILE_OR_PATH_ALREADY_EXIST, {}};
    }
    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
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
  static inline auto cpfileAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    return FileOperation::cpfile(pre, rel, to);
  }
  static auto cpfile(const QString& pre, const QString& rel, const QString& to) -> RETURN_TYPE {
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

    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
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

  static inline auto cpdirAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 3) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    const QString& to = parms[2];
    return FileOperation::cpdir(pre, rel, to);
  }

  static auto cpdir(const QString& pre, const QString& rel, const QString& to) -> RETURN_TYPE {
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
    FileOperation::BATCH_COMMAND_LIST_TYPE recoverList;
    auto mkRootPthRet = QDir(to).mkpath(rel);
    if (not mkRootPthRet) {
      qDebug("Failed QDir(%s).mkpath(%s)", to.toStdString().c_str(), rel.toStdString().c_str());
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
          qDebug("Failed QDir(%s).mkpath(%s)", toPth.toStdString().c_str(), toRel.toStdString().c_str());
          return {ErrorCode::UNKNOWN_ERROR, recoverList};
        }
        recoverList.append({"rmpath", toPth, toRel});
      } else {  // file
        auto cpRet = QFile(fromPth).copy(toPath);
        if (not cpRet) {
          qDebug("Failed QFile(%s).copy(%s)", fromPth.toStdString().c_str(), toPath.toStdString().c_str());
          return {ErrorCode::UNKNOWN_ERROR, recoverList};
        }
        recoverList.append({"rmfile", toPth, toRel});
      }
    }
    return {ErrorCode::OK, recoverList};
  }

  static inline auto touchAgent(const QStringList& parms) -> RETURN_TYPE {
    if (parms.size() != 2) {
      return {ErrorCode::OPERATION_PARMS_NOT_MATCH, {}};
    }
    const QString& pre = parms[0];
    const QString& rel = parms[1];
    return FileOperation::touch(pre, rel);
  }

  static auto touch(const QString& pre, const QString& rel) -> RETURN_TYPE {
    if (not QDir(pre).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    const QString& pth = QDir(pre).absoluteFilePath(rel);
    QFile textFile(pth);
    if (textFile.exists()) {
      return {ErrorCode::OK, {}};  // after all it exists
    }
    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
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
  static auto executer(const BATCH_COMMAND_LIST_TYPE& aBatch, BATCH_COMMAND_LIST_TYPE& srcCommand) -> EXECUTE_RETURN_TYPE {
    FileOperation::BATCH_COMMAND_LIST_TYPE recoverList;
    int failedCommandCnt = 0;
    for (int i = 0; i < aBatch.size(); ++i) {
      const QStringList& cmds = aBatch[i];
      if (cmds.isEmpty()) {
        continue;
      }
      const QString& k = cmds[0];  // operation name
      QStringList vals(cmds.cbegin() + 1, cmds.cend());
      RETURN_TYPE returnEle = FileOperation::LambdaTable[k](vals);
      int ret = returnEle.first;
      BATCH_COMMAND_LIST_TYPE recover = returnEle.second;
      if (ret != ErrorCode::OK) {
        ++failedCommandCnt;
        qDebug("%s(%d parm(s))", k.toStdString().c_str(), vals.size());
      }
      if (k == "moveToTrash" and not srcCommand.isEmpty()) {  // name in trashbin is now changed compared with last time in trashbin
        if (recover.size() > 1) {
          qDebug("moveToTrash recover command can only <= 1. Here is[%d]", recover.size());
          assert(false);
        }
        if (recover.size() == 1) {
          *(srcCommand.rbegin() + i) = recover[0];
        } else {
          (srcCommand.rbegin() + i)->clear();
        }
      }
      recoverList += recover;
    }

    if (failedCommandCnt != 0) {
      qDebug("Above %d command(s) failed.", failedCommandCnt);
    }
    // in-place reverse
    return {failedCommandCnt == 0, QList<QStringList>(recoverList.crbegin(), recoverList.crend())};
  }

  static inline auto linkAgent(const QStringList& parms) -> RETURN_TYPE {
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

  static inline auto link(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath) -> RETURN_TYPE {
    const QString pth = QDir(pre).absoluteFilePath(rel);
    if (not QFile::exists(pth)) {
      return {ErrorCode::SRC_INEXIST, {}};
    }
    if (not QDir(to).exists()) {
      return {ErrorCode::DST_DIR_INEXIST, {}};
    }
    QString toPath(QDir(to).absoluteFilePath(rel) + ".lnk");
    QFile toFile(toPath);

    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
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

  static inline auto unlinkAgent(const QStringList& parms) -> RETURN_TYPE {
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

  static inline auto unlink(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath) -> RETURN_TYPE {
    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
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
  static const QMap<QString, std::function<RETURN_TYPE(const QStringList& QStringList)>> LambdaTable;
};

#endif  // FILEOPERATION_H
