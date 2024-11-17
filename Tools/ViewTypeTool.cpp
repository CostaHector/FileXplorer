#include "ViewTypeTool.h"
#include <QAction>

namespace ViewTypeTool {
const char* GetViewTypeHumanFriendlyStr(ViewType viewType) {
  if ((int)viewType >= (int)ViewType::VIEW_TYPE_BUTT) {
    qDebug("viewType[%d] is invalid", (int)viewType);
    return "";
  }
  return VIEW_TYPE_2_STR[(int)viewType];
}

ViewType GetViewTypeByActionText(const QAction* viewAct) {
  if (viewAct == nullptr) {
    qWarning("viewAct is nullptr");
    return ViewType::VIEW_TYPE_BUTT;
  }
  const QString& viewTypeStr = viewAct->text();
  auto it = ACTION_TEXT_2_VIEW_TYPE.find(viewTypeStr);
  if (it == ACTION_TEXT_2_VIEW_TYPE.cend()) {
    qWarning("viewtype[%s] not found in %d element(s) map", qPrintable(viewTypeStr), ACTION_TEXT_2_VIEW_TYPE.size());
    return ViewType::VIEW_TYPE_BUTT;
  }
  return it.value();
}

}  // namespace ViewTypeTool
