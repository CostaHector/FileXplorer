#include "ViewTypeTool.h"
#include "PublicMacro.h"
#include <QAction>
#include <QMap>
#include <QString>

namespace ViewTypeTool {
const QMap<QString, ViewType> ACTION_TEXT_2_VIEW_TYPE  //
    {
        {ENUM_2_STR(LIST), ViewType::LIST},      //
        {ENUM_2_STR(TABLE), ViewType::TABLE},    //
        {ENUM_2_STR(TREE), ViewType::TREE},      //
        {ENUM_2_STR(SEARCH), ViewType::SEARCH},  //
        {ENUM_2_STR(MOVIE), ViewType::MOVIE},    //
        {ENUM_2_STR(SCENE), ViewType::SCENE},    //
        {ENUM_2_STR(CAST), ViewType::CAST},      //
        {ENUM_2_STR(JSON), ViewType::JSON}       //
    };

const char* GetViewTypeHumanFriendlyStr(ViewType viewType) {
  if (viewType < ViewType::VIEW_TYPE_BEGIN || viewType >= ViewType::VIEW_TYPE_BUTT) {
    return "unknown view type";
  }
  static char ViewType2CharArray[(int)ViewType::VIEW_TYPE_BUTT][10]{
      ENUM_2_STR(LIST),    //
      ENUM_2_STR(TABLE),   //
      ENUM_2_STR(TREE),    //
      ENUM_2_STR(SEARCH),  //
      ENUM_2_STR(MOVIE),   //
      ENUM_2_STR(SCENE),   //
      ENUM_2_STR(CAST),    //
      ENUM_2_STR(JSON),    //
  };
  return ViewType2CharArray[(int)viewType];
}

ViewType GetViewTypeByActionText(const QAction* pViewAct) {
  if (pViewAct == nullptr) {
    qWarning("pViewAct is nullptr");
    return ViewType::VIEW_TYPE_BUTT;
  }
  const QString& viewTypeStr = pViewAct->text();
  return ACTION_TEXT_2_VIEW_TYPE.value(viewTypeStr, ViewType::VIEW_TYPE_BUTT);
}

}  // namespace ViewTypeTool
