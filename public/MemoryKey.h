#ifndef MEMORYKEY_H
#define MEMORYKEY_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSet>
#include <QDir>

namespace VALUE_CHECKER_TYPE {
enum VALUE_TYPE {
  ERROR_TYPE = 0,
  PLAIN_STR = 1,
  PLAIN_BOOL = 2,
  PLAIN_INT = 3,
  PLAIN_FLOAT = 4,
  PLAIN_DOUBLE,
  FILE_PATH,
  EXT_SPECIFIED_FILE_PATH,
  FOLDER_PATH,
  RANGE_INT,
  SWITCH_STRING,  // "010101"
  CANDIDATE_STRING,
  QSTRING_LIST,
};
}

class ValueChecker {
 public:
  friend struct KV;

  explicit ValueChecker(const QStringList& candidates,  //
                        const VALUE_CHECKER_TYPE::VALUE_TYPE valueType_ = VALUE_CHECKER_TYPE::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH);

  explicit ValueChecker(int minV_ = INT32_MIN, int maxV_ = INT32_MAX);
  explicit ValueChecker(const QSet<QChar>& chars = {'0', '1'}, int minLength = 1);

  explicit ValueChecker(const VALUE_CHECKER_TYPE::VALUE_TYPE valueType_);

  static bool isFileExist(const QString& path);
  static bool isFolderExist(const QString& path);
  bool isStrInCandidate(const QString& str) const;
  bool isSpecifiedExtensionFileExist(const QString& path) const;
  bool isIntInRange(const int v) const;
  bool isSwitchString(const QString& switchs) const;

  bool operator()(const QVariant& v) const;
  QString valueToString(const QVariant& v) const;
  QVariant strToQVariant(const QString& v) const;

 private:
  VALUE_CHECKER_TYPE::VALUE_TYPE valueType;

  QSet<QString> m_strCandidates;  // e.g. extension candidates
  QSet<QChar> m_switchStates;
  int m_switchMinCnt;

  int minV;
  int maxV;
};

struct KV {
  explicit KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_);
  QSet<QString> GetCandidatePool() const { return checker.m_strCandidates; }

  QString valueToString() const;
  QString valueToString(const QVariant& v_) const;

  QString name;
  QVariant v;
  ValueChecker checker;
};

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
  static const KV SHOW_FOLDER_PREVIEW_HTML;
  static const KV FOLDER_PREVIEW_TYPE;
  static const KV SHOW_FLOATING_PREVIEW;

  static const KV SHOW_QUICK_NAVIGATION_TOOL_BAR;
  static const KV SHOW_FRAMELESS_WINDOW;
  static const KV EXPAND_OFFICE_STYLE_MENUBAR;
  static const KV QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA;

  static const KV ITEM_VIEW_FONT_SIZE;
  static const KV DEFAULT_VIDEO_PLAYER;
  static const KV DEFAULT_NEW_CHOICE;
  static const KV DEFAULT_COPY_CHOICE;
  static const KV DEFAULT_RENAME_CHOICE;
  static const KV DEFAULT_EXTRACT_CHOICE;
  static const KV MOVE_TO_PATH_HISTORY;
  static const KV COPY_TO_PATH_HISTORY;
  static const KV WHERE_CLAUSE_HISTORY;

  static const KV MENU_RIBBON_CURRENT_TAB_INDEX;
  static const KV COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT;

  static const KV TABLE_DEFAULT_SECTION_SIZE;
  static const KV TABLE_DEFAULT_COLUMN_SECTION_SIZE;
  static const KV PERFORMER_IMAGE_FIXED_HEIGHT;

  static const KV VIDS_LAST_TABLE_NAME;
  static const KV RIGHT_CLICK_TOOLBUTTON_STYLE;

  static const KV NAME_PATTERN_USED_CREATE_BATCH_FILES;
  static const KV NAME_PATTERN_USED_CREATE_BATCH_FOLDERS;

  static const KV DIR_FILTER_ON_SWITCH_ENABLE;

  static const KV VIDEO_PLAYER_VOLUME;
  static const KV VIDEO_PLAYER_MUTE;
  static const KV SEARCH_INCLUDING_SUBDIRECTORIES;
  static const KV SEARCH_NAME_CASE_SENSITIVE;
  static const KV SEARCH_CONTENTS_CASE_SENSITIVE;
  static const KV ADVANCE_SEARCH_LINEEDIT_VALUE;
  static const KV DISABLE_ENTRIES_DONT_PASS_FILTER;

  static const KV RENAMER_INCLUDING_FILE_EXTENSION;
  static const KV RENAMER_INCLUDING_DIR;
  static const KV RENAMER_OLD_STR_LIST;
  static const KV RENAMER_NEW_STR_LIST;
  static const KV RENAMER_INSERT_INDEXES_LIST;
  static const KV RENAMER_ARRANGE_SECTION_INDEX;
  static const KV RENAMER_REGEX_ENABLED;

  static const KV RENAMER_NUMERIAZER_START_INDEX;
  static const KV RENAMER_NUMERIAZER_NO_FORMAT;
  static const KV RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX;

  static const KV WIN32_MEDIAINFO_LIB_PATH;
  static const KV WIN32_PERFORMERS_TABLE;
  static const KV WIN32_AKA_PERFORMERS;
  static const KV WIN32_STANDARD_STUDIO_NAME;
  static const KV WIN32_TERMINAL_OPEN_BATCH_FILE_PATH;
  static const KV LOG_DEVEL_DEBUG;
  static const KV WIN32_RUNLOG;
  static const KV WIN32_RUND_IMG_PATH;

  static const KV LINUX_MEDIAINFO_LIB_PATH;

  static const KV LINUX_PERFORMERS_TABLE;
  static const KV LINUX_AKA_PERFORMERS;
  static const KV LINUX_STANDARD_STUDIO_NAME;
  static const KV LINUX_RUNLOG;
  static const KV LINUX_RUND_IMG_PATH;

  static const KV DUPLICATE_FINDER_DEVIATION_DURATION;  // 2s ~ 20s
  static const KV DUPLICATE_FINDER_DEVIATION_FILESIZE;  // 2kB ~ 30MB

  static const KV SHOW_HAR_IMAGE_PREVIEW;
};

#endif  // MEMORYKEY_H
