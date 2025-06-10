#ifndef FILEOPERATORPUB_H
#define FILEOPERATORPUB_H

#include "public/PublicMacro.h"
#include <QStringList>
namespace FileOperatorType {

#define FILE_OPERATOR_ERROR_FIELD_MAPPING                      \
  FILE_OPERATOR_ERROR_KEY_ITEM(OK)                             \
  FILE_OPERATOR_ERROR_KEY_ITEM(UNKNOWN_ERROR)                  \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_PRE_DIR_INEXIST)            \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_FILE_INEXIST)               \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_DIR_INEXIST)                \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_INEXIST)                    \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_DIR_INEXIST)                \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_PRE_DIR_CANNOT_MAKE)        \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FOLDER_ALREADY_EXIST)       \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FILE_ALREADY_EXIST)         \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FILE_OR_PATH_ALREADY_EXIST) \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_ITEM_ALREADY_EXIST)         \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_FILE)             \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_PATH)             \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_DIR)              \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_FOLDER_FORCE)     \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_MAKE_PATH)               \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_MAKE_DIR)                \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_MAKE_LINK)               \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_LINK_INEXIST)               \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_LINK)             \
  FILE_OPERATOR_ERROR_KEY_ITEM(OPERATION_NOT_AVAILABLE)        \
  FILE_OPERATOR_ERROR_KEY_ITEM(OPERATION_PARMS_NOT_MATCH)      \
  FILE_OPERATOR_ERROR_KEY_ITEM(RECYCLE_OCCUPIED_FILE_FAILED)   \
  FILE_OPERATOR_ERROR_KEY_ITEM(PATH_NAME_LIST_NOT_EQUAL)       \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_RENAME)                  \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_MV)                      \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_PRE_DIR_OCCUPIED_BY_FILE)   \
  FILE_OPERATOR_ERROR_KEY_ITEM(COMMAND_TYPE_UNSUPPORT)

enum ErrorCode {
#define FILE_OPERATOR_ERROR_KEY_ITEM(enu) enu,
  FILE_OPERATOR_ERROR_FIELD_MAPPING
#undef FILE_OPERATOR_ERROR_KEY_ITEM
      ERROR_BUTT
};  // namespace ErrorCode

const QString FILE_OPERATOR_ERROR_TO_STR[ERROR_BUTT + 1] = {
#define FILE_OPERATOR_ERROR_KEY_ITEM(enu) ENUM_2_STR(enu),
    FILE_OPERATOR_ERROR_FIELD_MAPPING
#undef FILE_OPERATOR_ERROR_KEY_ITEM
        ENUM_2_STR(ERROR_BUTT)  //
};

#define FILE_OPERATOR_TYPE_FIELD_MAPPING                                              \
  FILE_OPERATOR_KEY_ITEM(RMFILE, 0, rmfileAgent, TWO_PARAMS, TWO_ARGVS)               \
  FILE_OPERATOR_KEY_ITEM(RMPATH, 1, rmpathAgent, TWO_PARAMS, TWO_ARGVS)               \
  FILE_OPERATOR_KEY_ITEM(RMDIR, 2, rmdirAgent, TWO_PARAMS, TWO_ARGVS)                 \
  FILE_OPERATOR_KEY_ITEM(RMFOLDERFORCE, 3, rmFolderForceAgent, TWO_PARAMS, TWO_ARGVS) \
  FILE_OPERATOR_KEY_ITEM(MOVETOTRASH, 4, moveToTrashAgent, TWO_PARAMS, TWO_ARGVS)     \
  FILE_OPERATOR_KEY_ITEM(TOUCH, 5, touchAgent, TWO_PARAMS, TWO_ARGVS)                 \
  FILE_OPERATOR_KEY_ITEM(MKPATH, 6, mkpathAgent, TWO_PARAMS, TWO_ARGVS)               \
  FILE_OPERATOR_KEY_ITEM(MKDIR, 7, mkdirAgent, TWO_PARAMS, TWO_ARGVS)                 \
  FILE_OPERATOR_KEY_ITEM(RENAME, 8, renameAgent, THREE_PARAMS, THREE_ARGVS)           \
  FILE_OPERATOR_KEY_ITEM(MV, 9, mvAgent, THREE_PARAMS, THREE_ARGVS)                   \
  FILE_OPERATOR_KEY_ITEM(CPFILE, 10, cpfileAgent, THREE_PARAMS, THREE_ARGVS)          \
  FILE_OPERATOR_KEY_ITEM(CPDIR, 11, cpdirAgent, THREE_PARAMS, THREE_ARGVS)            \
  FILE_OPERATOR_KEY_ITEM(LINK, 12, linkAgent, THREE_PARAMS, THREE_ARGVS)              \
  FILE_OPERATOR_KEY_ITEM(UNLINK, 13, unlinkAgent, THREE_PARAMS, THREE_ARGVS)

enum FILE_OPERATOR_E {
  BEGIN,
#define FILE_OPERATOR_KEY_ITEM(enu, val, func, FACTORY_PARAM, FACTORY_ARGVS) enu = val,
  FILE_OPERATOR_TYPE_FIELD_MAPPING
#undef FILE_OPERATOR_KEY_ITEM
      OPERATOR_BUTT,
};

const QString FILE_OPERATOR_TO_STR[OPERATOR_BUTT + 1] = {
#define FILE_OPERATOR_KEY_ITEM(enu, val, func, FACTORY_PARAMS, FACTORY_ARGVS) ENUM_2_STR(enu),
    FILE_OPERATOR_TYPE_FIELD_MAPPING
#undef FILE_OPERATOR_KEY_ITEM
        ENUM_2_STR(OPERATOR_BUTT)  //
};

struct ACMD {
#define TWO_PARAMS const QString &pre, const QString &rel
#define TWO_ARGVS pre, rel
#define THREE_PARAMS const QString &pre, const QString &rel, const QString &to
#define THREE_ARGVS pre, rel, to

#define FILE_OPERATOR_KEY_ITEM(enu, val, func, FACTORY_PARAMS, FACTORY_ARGVS) \
  static ACMD GetInst##enu(FACTORY_PARAMS) { return {enu, {FACTORY_ARGVS}}; }
  FILE_OPERATOR_TYPE_FIELD_MAPPING  // Factory Mode
#undef FILE_OPERATOR_KEY_ITEM

#undef THREE_ARGVS
#undef THREE_PARAMS
#undef TWO_ARGVS
#undef TWO_PARAMS

      FILE_OPERATOR_E op;
  QStringList lst;

  int size() const { return lst.size(); }
  QString& operator[](int i) { return lst[i]; }

  void clear() {
    op = FILE_OPERATOR_E::OPERATOR_BUTT;
    lst.clear();
  };
  explicit operator bool() const {  //
    return !isEmpty();
  }

  bool isEmpty() const {  //
    return op == FILE_OPERATOR_E::OPERATOR_BUTT || lst.isEmpty();
  }

  QString toStr() const {  //
    return FILE_OPERATOR_TO_STR[op] + ":" + lst.join(';');
  }

  QString toStr(ErrorCode code) const {  //
    return '[' + FILE_OPERATOR_ERROR_TO_STR[code] + ']' + FILE_OPERATOR_TO_STR[op] + ":" + lst.join(';');
  }
  bool operator==(const ACMD& rhs) const;
};

using BATCH_COMMAND_LIST_TYPE = QList<ACMD>;

struct RETURN_TYPE {
  RETURN_TYPE(ErrorCode ret_ = ErrorCode::OK, const BATCH_COMMAND_LIST_TYPE& cmds_ = {})  //
      : ret{ret_}, cmds{cmds_} {}
  ErrorCode ret;
  BATCH_COMMAND_LIST_TYPE cmds;

  explicit operator bool() const { return ret == ErrorCode::OK; }
  int size() const { return cmds.size(); }
  bool isRecoverable() const { return !cmds.isEmpty(); };

  ACMD& operator[](int i) { return cmds[i]; }
  const ACMD& operator[](int i) const { return cmds[i]; }
};

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs);

QString BatchCommands2String(const BATCH_COMMAND_LIST_TYPE& cmds);

}  // namespace FileOperatorType

#endif  // FILEOPERATORPUB_H
