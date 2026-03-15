#include "AdvanceRenamerTestTool.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceRenamer.h"
#include "EndToExposePrivateMember.h"

namespace AdvanceRenamerTestTool {
QList<int> g_execReturnValue;

void set(const QList<int> newReturnValues) {
  g_execReturnValue = newReturnValues;
}

void clear() {
  (decltype(g_execReturnValue){}).swap(g_execReturnValue);
}

int execCoreMock(AdvanceRenamer* self) {
  if (self == nullptr) {
    return -1;
  }
  self->SetApplyResult(false);
  QDialogButtonBox* dlgBtnBox = self->m_buttonBox;
  if (dlgBtnBox == nullptr) {
    return -2;
  }
  if (g_execReturnValue.isEmpty()) {
    return -3;
  }
  int ansCode = g_execReturnValue.takeFirst();
  switch (ansCode) {
    case QDialog::DialogCode::Rejected: {
      emit dlgBtnBox->rejected();
      return QDialog::DialogCode::Rejected;
    }
    case QDialog::DialogCode::Accepted: {
      emit dlgBtnBox->accepted();
      self->SetApplyResult(true);
      return QDialog::DialogCode::Accepted;
    }
    default:
      return -4;
  }
}
}  // namespace AdvanceRenamerTestTool
