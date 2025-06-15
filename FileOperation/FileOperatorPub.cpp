#include "FileOperatorPub.h"
#include "public/MemoryKey.h"
namespace FileOperatorType {

BATCH_COMMAND_LIST_TYPE& operator+=(BATCH_COMMAND_LIST_TYPE& lhs, const RETURN_TYPE& rhs) {
  return lhs += rhs.cmds;
}

bool g_bReturnErrorCodeUponAnyFailure = true;
void InitReturnErrorCodeUponAnyFailureSw() {
  g_bReturnErrorCodeUponAnyFailure = PreferenceSettings().value(MemoryKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.name, MemoryKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.v).toBool();
}

bool IsReturnErrorCodeUponAnyFailureSw() {
  return g_bReturnErrorCodeUponAnyFailure;
}

void SetReturnErrorCodeUponAnyFailureSw(bool sw) {
  g_bReturnErrorCodeUponAnyFailure = sw;
  PreferenceSettings().setValue(MemoryKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.name, sw);
}

}  // namespace FileOperatorType
