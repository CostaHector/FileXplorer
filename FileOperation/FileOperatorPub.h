#ifndef FILEOPERATORPUB_H
#define FILEOPERATORPUB_H

#include <QStringList>
namespace FileOperatorType {

enum ErrorCode {
  UNKNOWN_ERROR = -1,
  OK = 0,
  SRC_PRE_DIR_INEXIST = 1,
  SRC_FILE_INEXIST = 2,
  SRC_DIR_INEXIST = 3,
  SRC_INEXIST = 4,
  DST_DIR_INEXIST = 5,
  DST_PRE_DIR_CANNOT_MAKE = 6,
  DST_FOLDER_ALREADY_EXIST = 7,
  DST_FILE_ALREADY_EXIST = 8,
  DST_FILE_OR_PATH_ALREADY_EXIST = 9,
  CANNOT_REMOVE_FILE = 10,
  CANNOT_REMOVE_DIR = 11,
  CANNOT_MAKE_LINK = 12,
  DST_LINK_INEXIST = 13,
  CANNOT_REMOVE_LINK = 14,
  OPERATION_NOT_AVAILABLE = 15,
  OPERATION_PARMS_NOT_MATCH = 16,
  RECYCLE_OCCUPIED_FILE_FAILED = 17,
  PATH_NAME_LIST_NOT_EQUAL = 18,
  COMMAND_TYPE_UNSUPPORT = 19
};  // namespace ErrorCode

enum FileOperator {
  RMFILE = 0,
  RMPATH,
  RMDIR,
  MOVETOTRASH,

  TOUCH,
  MKPATH,
  RENAME,
  CPFILE,

  CPDIR,
  LINK,
  UNLINK,
  BUTT
};

struct ACMD {
  FileOperator op;
  QStringList lst;
  int size() const { return lst.size(); }
  QString& operator[](int i) { return lst[i]; }
  void clear() {
    op = FileOperator::BUTT;
    lst.clear();
  };
  operator bool() const { return !isEmpty(); }
  bool isEmpty() const { return op == FileOperator::BUTT || lst.isEmpty(); }
  QString toStr() const { return QString::number(op) + ":" + lst.join('\t'); }
};

using BATCH_COMMAND_LIST_TYPE = QList<ACMD>;

struct RETURN_TYPE {
  int ret;
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
