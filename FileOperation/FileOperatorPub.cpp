#include "FileOperatorPub.h"
namespace FileOperatorType {

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

bool RETURN_TYPE::operator==(const RETURN_TYPE& rhs) const {
  return ret == rhs.ret && cmds == rhs.cmds;
}

bool RETURN_TYPE::operator!=(const RETURN_TYPE& rhs) const {
  return !(*this == rhs);
}

}  // namespace FileOperatorType
