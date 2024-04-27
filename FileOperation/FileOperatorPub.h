#ifndef FILEOPERATORPUB_H
#define FILEOPERATORPUB_H

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
constexpr int RECYCLE_OCCUPIED_FILE_FAILED = 17;
constexpr int PATH_NAME_LIST_NOT_EQUAL = 18;
constexpr int COMMAND_TYPE_UNSUPPORT = 19;
constexpr int UNKNOWN_ERROR = -1;
}  // namespace ErrorCode

#include <QStringList>
namespace FileOperatorType {
using BATCH_COMMAND_LIST_TYPE = QList<QStringList>;
using RETURN_TYPE = QPair<int, BATCH_COMMAND_LIST_TYPE>;
using EXECUTE_RETURN_TYPE = QPair<bool, BATCH_COMMAND_LIST_TYPE>;

QString BatchCommands2String(const BATCH_COMMAND_LIST_TYPE& cmds);
}  // namespace FileOperatorType

#endif  // FILEOPERATORPUB_H
