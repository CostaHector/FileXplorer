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
  KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_, bool bUserEditable=false);

  QString InitialValueToString() const;
  QString valueToString(const QVariant& v_) const;

  QString name;
  QVariant v;
private:
  static QList<const KV*> mEditableKVs;
  ValueChecker checker;
};

struct MemoryKey {
  static const KV DEFAULT_OPEN_PATH;
  static const KV LANGUAGE_ZH_CN;
  static const KV PATH_LAST_TIME_COPY_TO;
  static const KV PATH_JSON_EDITOR_LOAD_FROM;
  static const KV PATH_VIDEO_PLAYER_OPEN_PATH;
  static const KV PATH_PERFORMER_IMAGEHOST_LOCATE;
  static const KV PATH_DB_INSERT_VIDS_FROM;
  static const KV PATH_DB_INSERT_TORRENTS_FROM;

  static const KV KEEP_VIDEOS_PLAYLIST_SHOW;
  static const KV AUTO_PLAY_NEXT_VIDEO;
  static const KV FOLDER_PREVIEW_TYPE;
  static const KV SHOW_FLOATING_PREVIEW;

  static const KV SHOW_QUICK_NAVIGATION_TOOL_BAR;
  static const KV SHOW_FRAMELESS_WINDOW;
  static const KV EXPAND_OFFICE_STYLE_MENUBAR;
  static const KV QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA;

  static const KV ITEM_VIEW_FONT_SIZE;
  static const KV DEFAULT_NEW_CHOICE;
  static const KV DEFAULT_COPY_CHOICE;
  static const KV DEFAULT_RENAME_CHOICE;
  static const KV DEFAULT_THUMBNAILS_DIMENSION;
  static const KV DEFAULT_EXTRACT_CHOICE;
  static const KV MOVE_TO_PATH_HISTORY;
  static const KV COPY_TO_PATH_HISTORY;
  static const KV FILE_SYSTEM_STRUCTURE_WAY;
  static const KV WHERE_CLAUSE_HISTORY;

  static const KV MENU_RIBBON_CURRENT_TAB_INDEX;
  static const KV COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT;

  static const KV TABLE_DEFAULT_SECTION_SIZE;
  static const KV TABLE_DEFAULT_COLUMN_SECTION_SIZE;

  static const KV VIDS_LAST_TABLE_NAME;

  static const KV NAME_PATTERN_USED_CREATE_BATCH_FILES;
  static const KV NAME_PATTERN_USED_CREATE_BATCH_FOLDERS;

  static const KV DIR_FILTER_ON_SWITCH_ENABLE;

  static const KV VIDEO_PLAYER_VOLUME;
  static const KV VIDEO_PLAYER_MUTE;
  static const KV SEARCH_INCLUDING_SUBDIRECTORIES;
  static const KV SEARCH_NAME_CASE_SENSITIVE;
  static const KV SEARCH_CONTENTS_CASE_SENSITIVE;
  static const KV ADVANCE_SEARCH_LINEEDIT_VALUE;
  static const KV ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE;
  static const KV GRAY_ENTRIES_DONT_PASS_FILTER;
  static const KV ADVANCE_SEARCH_MODE;

  static const KV RENAMER_NAME_EXT_INDEPENDENT;
  static const KV RENAMER_INCLUDING_DIR;
  static const KV RENAMER_OLD_STR_LIST;
  static const KV RENAMER_NEW_STR_LIST;
  static const KV RENAMER_INSERT_INDEXES_LIST;
  static const KV RENAMER_ARRANGE_SECTION_INDEX;
  static const KV RENAMER_REGEX_ENABLED;

  static const KV RENAMER_NUMERIAZER_UNIQUE_EXT_COUNTER;
  static const KV RENAMER_NUMERIAZER_NO_FORMAT;
  static const KV RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX;

  static const KV ALL_LOG_LEVEL_AUTO_FFLUSH;

  static const KV DUPLICATE_FINDER_DEVIATION_DURATION;  // 2s ~ 20s
  static const KV DUPLICATE_FINDER_DEVIATION_FILESIZE;  // 2kB ~ 30MB

  static const KV SHOW_HAR_IMAGE_PREVIEW;
  static const KV RETURN_ERRORCODE_UPON_ANY_FAILURE;

  static const KV RATE_MOVIE_DEFAULT_VALUE;
};

struct BrowserKey {
  static const KV CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES;
  static const KV CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS;
  static const KV CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE;
  static const KV CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX;
  static const KV FLOATING_IMAGE_VIEW_SHOW;
  static const KV FLOATING_VIDEO_VIEW_SHOW;
  static const KV FLOATING_OTHER_VIEW_SHOW;
  static const KV FLOATING_MEDIA_TYPE_SEQ;
};

struct RedunImgFinderKey {
  static const KV ALSO_RECYCLE_EMPTY_IMAGE;
  static const KV RUND_IMG_PATH;
};

#endif  // MEMORYKEY_H
