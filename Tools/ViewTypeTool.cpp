#include "ViewTypeTool.h"
#include "PublicMacro.h"
#include <QAction>
#include <QMap>
#include <QString>

namespace ViewTypeTool {
const char* c_str(ViewType viewType) {
  if (viewType < ViewType::VIEW_TYPE_BEGIN || viewType >= ViewType::VIEW_TYPE_BUTT) {
    return "unknown view type";
  }
  static const char ViewType2CharArray[(int)ViewType::VIEW_TYPE_BUTT][10]{
#define VIEW_TYPE_ITEM(enu, val) ENUM_2_STR(enu),
    VIEW_TYPES_ENUM_VALUE_MAPPING
#undef VIEW_TYPE_ITEM
  };
  return ViewType2CharArray[(int)viewType];
}

ViewType GetViewTypeByActionText(const QAction* pViewAct) {
  if (pViewAct == nullptr) {
    qWarning("pViewAct is nullptr");
    return ViewType::VIEW_TYPE_BUTT;
  }
  static const QMap<QString, ViewType> ACTION_TEXT_2_VIEW_TYPE  //
      {
#define VIEW_TYPE_ITEM(enu, val) {ENUM_2_STR(enu), ViewType::enu},
          VIEW_TYPES_ENUM_VALUE_MAPPING
#undef VIEW_TYPE_ITEM
      };
  const QString& viewTypeStr = pViewAct->text();
  return ACTION_TEXT_2_VIEW_TYPE.value(viewTypeStr, ViewType::VIEW_TYPE_BUTT);
}

}  // namespace ViewTypeTool
