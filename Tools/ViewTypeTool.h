#ifndef TYPEVIEWSTOOL_H
#define TYPEVIEWSTOOL_H

class QAction;
#include <QString>
#include <QMap>
namespace ViewTypeTool {
enum class ViewType : char {
  VIEW_TYPE_BEGIN = 0,
  LIST = VIEW_TYPE_BEGIN,
  TABLE,
  TREE,
  SEARCH,
  MOVIE,
  SCENE,
  VIEW_TYPE_BUTT,
};

inline bool isFSView(ViewType vt) {
  return (vt == ViewType::LIST || vt == ViewType::TABLE || vt == ViewType::TREE);
}

const char* GetViewTypeHumanFriendlyStr(ViewType viewType);
ViewType GetViewTypeByActionText(const QAction* viewAct);

const char VIEW_TYPE_2_STR[(char)ViewType::VIEW_TYPE_BUTT][10]{"list", "table", "tree", "search", "movie", "scene"};
const QMap<QString, ViewType> ACTION_TEXT_2_VIEW_TYPE{{"list", ViewType::LIST},     {"table", ViewType::TABLE}, {"tree", ViewType::TREE},
                                                      {"search", ViewType::SEARCH}, {"movie", ViewType::MOVIE}, {"scene", ViewType::SCENE}};
}  // namespace ViewTypeTool

#endif  // TYPEVIEWSTOOL_H
