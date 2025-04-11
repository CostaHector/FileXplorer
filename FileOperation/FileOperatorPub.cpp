#include "FileOperatorPub.h"

namespace FileOperatorType {

bool ACMD::operator==(const ACMD& rhs) const {
  return op == rhs.op && lst == rhs.lst;
}

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

QString BatchCommands2String(const BATCH_COMMAND_LIST_TYPE& cmds) {
  QString s;  // don't print this in debug
  for (const auto& strLst : cmds) {
    s += strLst.toStr();
    s += '\n';
  }
  return s;
}

}  // namespace FileOperatorType
