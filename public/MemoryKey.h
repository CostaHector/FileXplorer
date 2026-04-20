#ifndef MEMORYKEY_H
#define MEMORYKEY_H

#include "ValueChecker.h"
#include <QSettings>

static inline QSettings& Configuration() {
#ifndef RUNNING_UNIT_TESTS
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", PROJECT_NAME);
#else
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "TestOnly", PROJECT_NAME);
#endif
  return settings;
}

class ConfigsModel;
class KV {
 public:
  friend class ConfigsModel;
  KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_, bool bUserEditable = false);

  QString InitialValueToString() const;
  QString valueToString(const QVariant& v_) const;
  bool isPath() const { return checker.isPath(); }

  QString name;
  QVariant v;

 private:
  static QList<const KV*> mEditableKVs;
  ValueChecker checker;
};

struct MemoryKey {
  static const KV LANGUAGE_ZH_CN;
  static const KV ROW_HEIGHT;
};

struct PathKey {
  static const KV STARTUP_PATH;
  static const KV LAST_TIME_COPY_TO;
  static const KV JSON_EDITOR_LOAD_FROM;
  static const KV VIDEO_PLAYER_OPEN_PATH;
  static const KV PERFORMER_IMAGEHOST_LOCATE;
  static const KV DB_INSERT_VIDS_FROM;
  static const KV DB_INSERT_TORRENTS_FROM;
};

struct BehaviorKey {
  static const KV NEW_CHOICE;
  static const KV COPY_CHOICE;
  static const KV RENAME_CHOICE;
  static const KV THUMBNAILS_DIMENSION;
  static const KV EXTRACT_CHOICE;

  static const KV MOVE_TO_HISTORY;
  static const KV COPY_TO_HISTORY;
  static const KV FILESYSTEM_STRUCTURE;

  static const KV VIDS_LAST_TABLE_NAME;
  static const KV WHERE_CLAUSE_HISTORY;

  static const KV BATCH_FILES_NAME_PATTERN;
  static const KV BATCH_FOLDERS_NAME_PATTERN;

  static const KV DIR_FILTER_ON_SWITCH_ENABLE;

  static const KV ALL_LOG_LEVEL_AUTO_FFLUSH;
  static const KV RETURN_ERRORCODE_UPON_ANY_FAILURE;
};

struct SearchKey {
  static const KV INCLUDING_SUBDIRECTORIES;
  static const KV NAME_CASE_SENSITIVE;
  static const KV CONTENTS_CASE_SENSITIVE;
  static const KV GRAY_ENTRIES_DONT_PASS_FILTER;
  static const KV ADVANCE_LINEEDIT_VALUE;
  static const KV ADVANCE_CONTENTS_LINEEDIT_VALUE;
  static const KV ADVANCE_MODE;
};

struct CompoVisKey {
  static const KV MENU_RIBBON_CURRENT_TAB_INDEX;
  static const KV FOLDER_PREVIEW_TYPE;
  static const KV SHOW_PREVIEW_DOCKER;
  static const KV SHOW_NAVIGATION_SIDEBAR;
  static const KV SHOW_HAR_IMAGE_PREVIEW;
  static const KV EXPAND_NAVIGATION_SIDEBAR;
  static const KV EXPAND_OFFICE_STYLE_MENUBAR;
};

struct SceneKey {
  static const KV SORT_ORDER_REVERSE;
  static const KV SORT_BY_ROLE;
  static const KV DISABLE_IMAGE_DECORATION;
  static const KV CNT_EACH_PAGE;
};

struct BrowserKey {
  static const KV CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES;
  static const KV CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS;
  static const KV FLOATING_IMAGE_VIEW_SHOW;
  static const KV FLOATING_VIDEO_VIEW_SHOW;
  static const KV FLOATING_OTHER_VIEW_SHOW;
  static const KV FLOATING_MEDIA_TYPE_SEQ;
};

struct RedunImgFinderKey {
  static const KV ALSO_RECYCLE_EMPTY_IMAGE;
  static const KV RUND_IMG_PATH;
};

struct FavoritesNavigationKey {
  static const KV SORT_BY_ROLE;
  static const KV SORT_ORDER_REVERSE;
};

struct VideoPlayerKey {
  static const KV VOLUME;
  static const KV MUTE;
  static const KV PLAYBACK_MODE;
  static const KV PLAYBACK_TRIGGER_MODE;
  static const KV AUTO_HIDE_TOOLBAR;

  static const KV DUPLICATE_FINDER_DEVIATION_DURATION;  // 2s ~ 20s
  static const KV DUPLICATE_FINDER_DEVIATION_FILESIZE;  // 2kB ~ 30MB

  static const KV RATE_MOVIE_DEFAULT_VALUE;
};

struct RenamerKey {
  static const KV NAME_EXT_INDEPENDENT;
  static const KV INCLUDING_DIR;
  static const KV OLD_STR_LIST;
  static const KV NEW_STR_LIST;
  static const KV INSERT_INDEXES_LIST;
  static const KV ARRANGE_SECTION_INDEX;
  static const KV REGEX_ENABLED;

  static const KV NUMERIAZER_UNIQUE_EXT_COUNTER;
  static const KV NUMERIAZER_NO_FORMAT;
  static const KV NUMERIAZER_NO_FORMAT_DEFAULT_INDEX;
};

namespace JsonOpMemoryKey {
const KV& STUDIO_OPERATION();
const KV& CAST_OPERATION();
const KV& TAGS_OPERATION();
}  // namespace JsonOpMemoryKey

#endif  // MEMORYKEY_H
