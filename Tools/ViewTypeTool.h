#ifndef TYPEVIEWSTOOL_H
#define TYPEVIEWSTOOL_H

class QAction;

namespace ViewTypeTool {
enum class ViewType {
  VIEW_TYPE_BEGIN = 0,
  // add after BEGIN
  LIST = VIEW_TYPE_BEGIN,
  TABLE,
  TREE,
  SEARCH,
  MOVIE,
  SCENE,
  CAST,
  JSON,
  // add before BUTT
  VIEW_TYPE_BUTT,
};

inline bool isFSView(ViewType vt) {
  return (vt == ViewType::LIST || vt == ViewType::TABLE || vt == ViewType::TREE);
}

inline bool IsUseFileSystemSearchBar(ViewType vt) {
  return (vt == ViewType::LIST || vt == ViewType::TABLE || vt == ViewType::TREE || vt == ViewType::JSON);
}

const char* GetViewTypeHumanFriendlyStr(ViewType viewType);

ViewType GetViewTypeByActionText(const QAction* pViewAct);
}  // namespace ViewTypeTool

#endif  // TYPEVIEWSTOOL_H
