#ifndef TYPEVIEWSTOOL_H
#define TYPEVIEWSTOOL_H

class QAction;
#include <QString>
namespace ViewTypeTool {
enum ViewType {
  LIST = 0,
  TABLE,
  TREE,
  SEARCH,
  MOVIE,
  SCENE,
  CAST,
  VIEW_TYPE_BUTT,
};

inline bool isFSView(ViewType vt) {
  return (vt == ViewType::LIST || vt == ViewType::TABLE || vt == ViewType::TREE);
}

const char* GetViewTypeHumanFriendlyStr(ViewType viewType);
ViewType GetViewTypeByActionText(const QAction* viewAct);
}  // namespace ViewTypeTool

#endif  // TYPEVIEWSTOOL_H
