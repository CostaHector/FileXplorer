#ifndef TYPEVIEWSTOOL_H
#define TYPEVIEWSTOOL_H

#define VIEW_TYPES_ENUM_VALUE_MAPPING \
VIEW_TYPE_ITEM(LIST,   0)    \
    VIEW_TYPE_ITEM(TABLE,  1)    \
    VIEW_TYPE_ITEM(TREE,   2)    \
    VIEW_TYPE_ITEM(SEARCH, 3)    \
    VIEW_TYPE_ITEM(MOVIE,  4)    \
    VIEW_TYPE_ITEM(SCENE,  5)    \
    VIEW_TYPE_ITEM(CAST,   6)    \
    VIEW_TYPE_ITEM(JSON,   7)    \

    class QAction;

namespace ViewTypeTool {
enum class ViewType {
  VIEW_TYPE_BEGIN = 0,
// add after BEGIN
#define VIEW_TYPE_ITEM(enu, val) enu = val,
  VIEW_TYPES_ENUM_VALUE_MAPPING
#undef VIEW_TYPE_ITEM
      // add before BUTT
      VIEW_TYPE_BUTT,
};

enum class ViewTypeMask {
#define VIEW_TYPE_ITEM(enu, val) enu = 1 << val,
  VIEW_TYPES_ENUM_VALUE_MAPPING
#undef VIEW_TYPE_ITEM
};

typedef int ViewTypeMaskInt;
namespace ViewTypeUtils {
constexpr ViewTypeMaskInt FS_VIEWS_MASK {(1 << (int)ViewType::LIST) | (1 << (int)ViewType::TABLE) | (1 << (int)ViewType::TREE)};
constexpr ViewTypeMaskInt VIEWS_USE_FILE_SYSTEM_SEARCHBAR { FS_VIEWS_MASK | (1 << (int)ViewType::SCENE) | (1 << (int)ViewType::JSON)};
constexpr ViewTypeMaskInt CHROME_SEARCH_MASK {FS_VIEWS_MASK | (1 << (int)ViewType::CAST) | (1 << (int)ViewType::JSON)};
constexpr ViewTypeMaskInt OPEN_IN_TERMINAL_MASK {FS_VIEWS_MASK | (1 << (int)ViewType::SEARCH) | (1 << (int)ViewType::CAST)};
constexpr ViewTypeMaskInt DECOMPRESS_MASK {FS_VIEWS_MASK | (1 << (int)ViewType::SEARCH)};
constexpr ViewTypeMaskInt PLAY_VIDEOS_MASK {FS_VIEWS_MASK | (1 << (int)ViewType::SEARCH)| (1 << (int)ViewType::MOVIE)};
}

inline bool IsMatch(ViewType vt, ViewTypeMaskInt candidates) { // ViewTypeMask
  return ((1 << (int)vt) & candidates) != 0;
}

inline bool isFSView(ViewType vt) {
  return IsMatch(vt, ViewTypeUtils::FS_VIEWS_MASK);
}

inline bool IsUseFileSystemSearchBar(ViewType vt) { // share the same filesystem search bar
  return IsMatch(vt, ViewTypeUtils::VIEWS_USE_FILE_SYSTEM_SEARCHBAR);
}

inline bool IsChromeSearchAvail(ViewType vt) {
  return IsMatch(vt, ViewTypeUtils::CHROME_SEARCH_MASK);
}

inline bool IsOpenInTerminalAvail(ViewType vt) {
  return IsMatch(vt, ViewTypeUtils::OPEN_IN_TERMINAL_MASK);
}

inline bool IsDecompressHereAvail(ViewType vt) {
  return IsMatch(vt, ViewTypeUtils::DECOMPRESS_MASK);
}

inline bool IsOpenVideosAvail(ViewType vt) {
  return IsMatch(vt, ViewTypeUtils::PLAY_VIDEOS_MASK);
}

const char* c_str(ViewType viewType);

ViewType GetViewTypeByActionText(const QAction* pViewAct);
}  // namespace ViewTypeTool

#endif  // TYPEVIEWSTOOL_H
