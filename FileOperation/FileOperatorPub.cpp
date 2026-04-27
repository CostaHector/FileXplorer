#include "FileOperatorPub.h"
#include "BehaviorKey.h"
#include "Configuration.h"
namespace FileOperatorType {

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

bool g_bReturnErrorCodeUponAnyFailure = true;
void InitReturnErrorCodeUponAnyFailureSw() {
  g_bReturnErrorCodeUponAnyFailure = getConfig(BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE).toBool();
}

bool IsReturnErrorCodeUponAnyFailureSw() {
  return g_bReturnErrorCodeUponAnyFailure;
}

void SetReturnErrorCodeUponAnyFailureSw(bool sw) {
  g_bReturnErrorCodeUponAnyFailure = sw;
  setConfig(BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE, sw);
}


bool RETURN_TYPE::operator==(const RETURN_TYPE& rhs) const {
  return ret == rhs.ret && cmds == rhs.cmds;
}

bool RETURN_TYPE::operator!=(const RETURN_TYPE& rhs) const {
  return !(*this == rhs);
}

}  // namespace FileOperatorType
