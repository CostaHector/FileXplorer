#ifndef COPYSTRINGLISTTOCLIPBOARD_H
#define COPYSTRINGLISTTOCLIPBOARD_H

#include <QString>

namespace CopyStringListToClipboard {
  bool PathStringListCopy(const QStringList& lst, const QString& opName);
}

#endif  // COPYSTRINGLISTTOCLIPBOARD_H
