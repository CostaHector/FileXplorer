#include "FileOperatorPub.h"

namespace FileOperatorType {

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

}  // namespace FileOperatorType
