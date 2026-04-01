#include "MockFriendlyTool.h"
#include <QInputDialog>

namespace MockFriendlyTool {

std::pair<int, bool> getInt(QWidget* parent,
                            const QString& title,
                            const QString& label,
                            int value,
                            int minValue,
                            int maxValue,
                            int step,
                            bool* ok,
                            Qt::WindowFlags flags) {
  bool bOk{false};
  int val = QInputDialog::getInt(parent, title, label, value, minValue, maxValue, step, &bOk, flags);
  return std::make_pair(val, bOk);
}

}  // namespace MockFriendlyTool
