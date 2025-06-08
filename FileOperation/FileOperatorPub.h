#ifndef FILEOPERATORPUB_H
#define FILEOPERATORPUB_H

#include "public/PublicMacro.h"
#include <QStringList>
namespace FileOperatorType {

#define FILE_OPERATOR_ERROR_FIELD_MAPPING                         \
  FILE_OPERATOR_ERROR_KEY_ITEM(OK, 0)                             \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_PRE_DIR_INEXIST, 1)            \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_FILE_INEXIST, 2)               \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_DIR_INEXIST, 3)                \
  FILE_OPERATOR_ERROR_KEY_ITEM(SRC_INEXIST, 4)                    \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_DIR_INEXIST, 5)                \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_PRE_DIR_CANNOT_MAKE, 6)        \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FOLDER_ALREADY_EXIST, 7)       \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FILE_ALREADY_EXIST, 8)         \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_FILE_OR_PATH_ALREADY_EXIST, 9) \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_FILE, 10)            \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_DIR, 11)             \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_MAKE_LINK, 12)              \
  FILE_OPERATOR_ERROR_KEY_ITEM(DST_LINK_INEXIST, 13)              \
  FILE_OPERATOR_ERROR_KEY_ITEM(CANNOT_REMOVE_LINK, 14)            \
  FILE_OPERATOR_ERROR_KEY_ITEM(OPERATION_NOT_AVAILABLE, 15)       \
  FILE_OPERATOR_ERROR_KEY_ITEM(OPERATION_PARMS_NOT_MATCH, 16)     \
  FILE_OPERATOR_ERROR_KEY_ITEM(RECYCLE_OCCUPIED_FILE_FAILED, 17)  \
  FILE_OPERATOR_ERROR_KEY_ITEM(PATH_NAME_LIST_NOT_EQUAL, 18)      \
  FILE_OPERATOR_ERROR_KEY_ITEM(COMMAND_TYPE_UNSUPPORT, 19)        \
  FILE_OPERATOR_ERROR_KEY_ITEM(UNKNOWN_ERROR, 20)

enum ErrorCode {
#define FILE_OPERATOR_ERROR_KEY_ITEM(enu, val) enu = val,
  FILE_OPERATOR_ERROR_FIELD_MAPPING
#undef FILE_OPERATOR_ERROR_KEY_ITEM
      ERROR_BUTT
};  // namespace ErrorCode

const QString FILE_OPERATOR_ERROR_TO_STR[ERROR_BUTT + 1] = {
#define FILE_OPERATOR_ERROR_KEY_ITEM(enu, val) ENUM_2_STR(enu),
    FILE_OPERATOR_ERROR_FIELD_MAPPING
#undef FILE_OPERATOR_ERROR_KEY_ITEM
        ENUM_2_STR(ERROR_BUTT)  //
};

#define FILE_OPERATOR_TYPE_FIELD_MAPPING                   \
  FILE_OPERATOR_KEY_ITEM(RMFILE, 0, rmfileAgent)           \
  FILE_OPERATOR_KEY_ITEM(RMPATH, 1, rmpathAgent)           \
  FILE_OPERATOR_KEY_ITEM(RMDIR, 2, rmdirAgent)             \
  FILE_OPERATOR_KEY_ITEM(MOVETOTRASH, 3, moveToTrashAgent) \
  FILE_OPERATOR_KEY_ITEM(TOUCH, 4, touchAgent)             \
  FILE_OPERATOR_KEY_ITEM(MKPATH, 5, mkpathAgent)           \
  FILE_OPERATOR_KEY_ITEM(RENAME, 6, renameAgent)           \
  FILE_OPERATOR_KEY_ITEM(CPFILE, 7, cpfileAgent)           \
  FILE_OPERATOR_KEY_ITEM(CPDIR, 8, cpdirAgent)             \
  FILE_OPERATOR_KEY_ITEM(LINK, 9, linkAgent)               \
  FILE_OPERATOR_KEY_ITEM(UNLINK, 10, unlinkAgent)

enum FILE_OPERATOR_E {
  BEGIN,
#define FILE_OPERATOR_KEY_ITEM(enu, val, func) enu = val,
  FILE_OPERATOR_TYPE_FIELD_MAPPING
#undef FILE_OPERATOR_KEY_ITEM
      OPERATOR_BUTT,
};

const QString FILE_OPERATOR_TO_STR[OPERATOR_BUTT + 1] = {
#define FILE_OPERATOR_KEY_ITEM(enu, val, func) ENUM_2_STR(enu),
    FILE_OPERATOR_TYPE_FIELD_MAPPING
#undef FILE_OPERATOR_KEY_ITEM
        ENUM_2_STR(OPERATOR_BUTT)  //
};

struct ACMD {
  FILE_OPERATOR_E op;
  QStringList lst;
  int size() const { return lst.size(); }
  QString& operator[](int i) { return lst[i]; }
  void clear() {
    op = FILE_OPERATOR_E::OPERATOR_BUTT;
    lst.clear();
  };
  operator bool() const {  //
    return !isEmpty();
  }
  bool isEmpty() const {  //
    return op == FILE_OPERATOR_E::OPERATOR_BUTT || lst.isEmpty();
  }
  QString toStr() const {  //
    return FILE_OPERATOR_TO_STR[op] + ":" + lst.join('\t');
  }
  QString toStr(ErrorCode code) const {  //
    return FILE_OPERATOR_ERROR_TO_STR[code] + '\t' + FILE_OPERATOR_TO_STR[op] + ":" + lst.join('\t');
  }
  bool operator==(const ACMD& rhs) const;
};

using BATCH_COMMAND_LIST_TYPE = QList<ACMD>;

struct RETURN_TYPE {
  ErrorCode ret;
  BATCH_COMMAND_LIST_TYPE cmds;
  operator bool() const { return ret == ErrorCode::OK; }
  int size() const { return cmds.size(); }
  ACMD& operator[](int i) { return cmds[i]; }
  const ACMD& operator[](int i) const { return cmds[i]; }
};

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs);

QString BatchCommands2String(const BATCH_COMMAND_LIST_TYPE& cmds);

}  // namespace FileOperatorType

#endif  // FILEOPERATORPUB_H
