#ifndef ADVANCERENAMERTESTTOOL_H
#define ADVANCERENAMERTESTTOOL_H

#include <QList>
class AdvanceRenamer;

namespace AdvanceRenamerTestTool {
void set(const QList<int> newReturnValues);
void clear();
int execCoreMock(AdvanceRenamer* self);
}  // namespace AdvanceRenamerTestTool

#endif  // ADVANCERENAMERTESTTOOL_H
