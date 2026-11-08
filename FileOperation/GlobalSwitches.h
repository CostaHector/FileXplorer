#ifndef GLOBALSWITCHES_H
#define GLOBALSWITCHES_H

namespace GlobalSwitches {
void InitReturnErrorCodeUponAnyFailureSw();
bool IsReturnErrorCodeUponAnyFailureSw();
void SetReturnErrorCodeUponAnyFailureSw(bool sw);

void InitConfirmBeforeMoveToTrashSw();
bool IsConfirmBeforeMoveToTrash();
void SetConfirmBeforeBeforeMoveToTrashSw(bool sw);
}
#endif // GLOBALSWITCHES_H
