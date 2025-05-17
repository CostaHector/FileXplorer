#include "ViewTypeTool.h"
#include "public/PublicMacro.h"
#include <QAction>
#include <QMap>

namespace ViewTypeTool {
const QMap<QString, ViewType> ACTION_TEXT_2_VIEW_TYPE  //
    {
        {ENUM_2_STR(LIST), ViewType::LIST},      //
        {ENUM_2_STR(TABLE), ViewType::TABLE},    //
        {ENUM_2_STR(TREE), ViewType::TREE},      //
        {ENUM_2_STR(SEARCH), ViewType::SEARCH},  //
        {ENUM_2_STR(MOVIE), ViewType::MOVIE},    //
        {ENUM_2_STR(SCENE), ViewType::SCENE},    //
        {ENUM_2_STR(CAST), ViewType::CAST},       //
        {ENUM_2_STR(JSON), ViewType::JSON}       //
    };

const char* GetViewTypeHumanFriendlyStr(ViewType viewType) {
  switch (viewType) {
    CASE_BRANCH_ENUM_TO_STRING(LIST);
    CASE_BRANCH_ENUM_TO_STRING(TABLE);
    CASE_BRANCH_ENUM_TO_STRING(TREE);
    CASE_BRANCH_ENUM_TO_STRING(SEARCH);
    CASE_BRANCH_ENUM_TO_STRING(MOVIE);
    CASE_BRANCH_ENUM_TO_STRING(SCENE);
    CASE_BRANCH_ENUM_TO_STRING(CAST);
    CASE_BRANCH_ENUM_TO_STRING(JSON);
    default: {
      qWarning("viewType[%d] is invalid", viewType);
      return "";
    }
  }
}

ViewType GetViewTypeByActionText(const QAction* viewAct) {
  if (viewAct == nullptr) {
    qWarning("viewAct is nullptr");
    return VIEW_TYPE_BUTT;
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
