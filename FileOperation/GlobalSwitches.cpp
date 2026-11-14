#include "GlobalSwitches.h"
#include "BehaviorKey.h"
#include "Configuration.h"

namespace GlobalSwitches {
bool g_bReturnErrorCodeUponAnyFailure = BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE.v.data.b;
bool g_bConfirmBeforeMoveToTrash = BehaviorKey::CONFIRM_BEFORE_MOVE_TO_TRASH.v.data.b;

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

void InitConfirmBeforeMoveToTrashSw() {
  g_bConfirmBeforeMoveToTrash = getConfig(BehaviorKey::CONFIRM_BEFORE_MOVE_TO_TRASH).toBool();
}
bool IsConfirmBeforeMoveToTrash() {
  return g_bConfirmBeforeMoveToTrash;
}
void SetConfirmBeforeBeforeMoveToTrashSw(bool sw) {
  g_bConfirmBeforeMoveToTrash = sw;
  setConfig(BehaviorKey::CONFIRM_BEFORE_MOVE_TO_TRASH, sw);
}
}
