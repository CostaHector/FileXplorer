#include "CopyStringListToClipboard.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QApplication>
#include <QClipboard>

namespace CopyStringListToClipboard{
bool PathStringListCopy(const QStringList& lst, const QString& opName) {
  if (lst.isEmpty()) {
    LOG_W("NOTHING %s copied. clipboard state unchange.", qPrintable(opName));
    return true;
  }
  const QString& copiedStr = lst.join('\n');
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  CHECK_NULLPTR_RETURN_FALSE(cb);
  LOG_D("[%d] letter(s) has been [%s].", lst.size(), qPrintable(opName));
  return true;
}
}
