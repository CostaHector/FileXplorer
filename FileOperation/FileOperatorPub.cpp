#include "FileOperatorPub.h"

namespace FileOperatorType {

bool ACMD::operator==(const ACMD& rhs) const {
  return op == rhs.op && lst == rhs.lst;
}

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

}  // namespace FileOperatorType
