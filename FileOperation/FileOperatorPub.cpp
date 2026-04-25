#include "FileOperatorPub.h"
#include "MemoryKey.h"
#include "Configuration.h"
namespace FileOperatorType {

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

bool g_bReturnErrorCodeUponAnyFailure = true;
void InitReturnErrorCodeUponAnyFailureSw() {
  g_bReturnErrorCodeUponAnyFailure = Configuration().value(BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.name, BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.toVariant()).toBool();
}

bool IsReturnErrorCodeUponAnyFailureSw() {
  return g_bReturnErrorCodeUponAnyFailure;
}

void SetReturnErrorCodeUponAnyFailureSw(bool sw) {
  g_bReturnErrorCodeUponAnyFailure = sw;
  Configuration().setValue(BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.name, sw);
}


bool RETURN_TYPE::operator==(const RETURN_TYPE& rhs) const {
  return ret == rhs.ret && cmds == rhs.cmds;
}

bool RETURN_TYPE::operator!=(const RETURN_TYPE& rhs) const {
  return !(*this == rhs);
}

}  // namespace FileOperatorType
