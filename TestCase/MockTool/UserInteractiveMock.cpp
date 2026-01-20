#include "UserInteractiveMock.h"

namespace UserInteractiveMock {

namespace InputDialog {

QString invoke_getItem(QWidget *parent,
                       const QString &title,
                       const QString &label,
                       const QStringList &items,
                       int current,
                       bool editable,
                       bool *ok,
                       Qt::WindowFlags flags,
                       Qt::InputMethodHints inputMethodHints) {
  std::pair<bool, QString> ok2returnValue = getItem_set();
  if (ok != nullptr) {
    *ok = ok2returnValue.first;
  }
  return ok2returnValue.second;
}

}

}
