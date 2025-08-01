﻿#ifndef MEMORYKEY_H
#define MEMORYKEY_H

#include "Memory/KV.h"
#include <QSettings>

static inline QSettings& PreferenceSettings() {
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", "FileExplorerReadOnly");
  return settings;
}

struct MemoryKey {
  static const KV DEFAULT_OPEN_PATH;
  static const KV LANGUAGE_ZH_CN;
  static const KV BACKGROUND_IMAGE;
  static const KV SHOW_BACKGOUND_IMAGE;
  static const KV PATH_LAST_TIME_COPY_TO;
  static const KV PATH_JSON_EDITOR_LOAD_FROM;
  static const KV PATH_VIDEO_PLAYER_OPEN_PATH;
  static const KV PATH_PERFORMER_IMAGEHOST_LOCATE;
  static const KV PATH_DB_INSERT_VIDS_FROM;
  static const KV PATH_DB_INSERT_TORRENTS_FROM;

  static const KV KEEP_VIDEOS_PLAYLIST_SHOW;
  static const KV AUTO_PLAY_NEXT_VIDEO;
  static const KV SHOW_FOLDER_PREVIEW;
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
  static const KV DEFAULT_THUMBNAIL_SAMPLE_PERIOD;
  static const KV DEFAULT_EXTRACT_CHOICE;
  static const KV MOVE_TO_PATH_HISTORY;
  static const KV COPY_TO_PATH_HISTORY;
  static const KV FILE_SYSTEM_STRUCTURE_WAY;
  static const KV WHERE_CLAUSE_HISTORY;

  static const KV MENU_RIBBON_CURRENT_TAB_INDEX;
  static const KV COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT;

  static const KV TABLE_DEFAULT_SECTION_SIZE;
  static const KV TABLE_DEFAULT_COLUMN_SECTION_SIZE;
  static const KV PERFORMER_IMAGE_FIXED_HEIGHT;

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
  static const KV DISABLE_ENTRIES_DONT_PASS_FILTER;
  static const KV ADVANCE_SEARCH_MODE;

  static const KV RENAMER_NAME_EXT_INDEPENDENT;
  static const KV RENAMER_INCLUDING_DIR;
  static const KV RENAMER_OLD_STR_LIST;
  static const KV RENAMER_NEW_STR_LIST;
  static const KV RENAMER_INSERT_INDEXES_LIST;
  static const KV RENAMER_ARRANGE_SECTION_INDEX;
  static const KV RENAMER_REGEX_ENABLED;

  static const KV RENAMER_NUMERIAZER_NO_FORMAT;
  static const KV RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX;

  static const KV WIN32_MEDIAINFO_LIB_PATH;
  static const KV WIN32_PERFORMERS_TABLE;
  static const KV WIN32_AKA_PERFORMERS;
  static const KV WIN32_STANDARD_STUDIO_NAME;
  static const KV WIN32_TERMINAL_OPEN_BATCH_FILE_PATH;
  static const KV LOG_LEVEL_PRINT_INSTANTLY;
  static const KV WIN32_RUNLOG;

  static const KV LINUX_PERFORMERS_TABLE;
  static const KV LINUX_AKA_PERFORMERS;
  static const KV LINUX_STANDARD_STUDIO_NAME;
  static const KV LINUX_RUNLOG;

  static const KV DUPLICATE_FINDER_DEVIATION_DURATION;  // 2s ~ 20s
  static const KV DUPLICATE_FINDER_DEVIATION_FILESIZE;  // 2kB ~ 30MB

  static const KV SHOW_HAR_IMAGE_PREVIEW;
  static const KV RETURN_ERRORCODE_UPON_ANY_FAILURE;
};


struct RedunImgFinderKey {
static const KV GEOMETRY;
static const KV ALSO_RECYCLE_EMPTY_IMAGE;
static const KV WIN32_RUND_IMG_PATH;
static const KV LINUX_RUND_IMG_PATH;
};

#endif  // MEMORYKEY_H
