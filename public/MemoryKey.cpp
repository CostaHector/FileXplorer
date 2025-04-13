#include "MemoryKey.h"
#include "PathTool.h"

using namespace VALUE_CHECKER_TYPE;

ValueChecker::ValueChecker(const QStringList& candidates, const VALUE_TYPE valueType_) : valueType{valueType_}, m_strCandidates{candidates.cbegin(), candidates.cend()} {}

ValueChecker::ValueChecker(int minV_, int maxV_) : valueType{RANGE_INT}, minV{minV_}, maxV{maxV_} {}

ValueChecker::ValueChecker(const QSet<QChar>& chars, int minLength) : valueType{SWITCH_STRING}, m_switchStates{chars}, m_switchMinCnt{minLength} {}

ValueChecker::ValueChecker(const VALUE_TYPE valueType_) : valueType(valueType_) {}

bool ValueChecker::isFileExist(const QString& path) {
  return QFileInfo(path).isFile();
}

bool ValueChecker::isFolderExist(const QString& path) {
  return QFileInfo(path).isDir();
}

bool ValueChecker::isStrInCandidate(const QString& str) const {
  return m_strCandidates.isEmpty() or m_strCandidates.contains(str);
}

bool ValueChecker::isSpecifiedExtensionFileExist(const QString& path) const {
  const QString& ext = PATHTOOL::GetFileExtension(path);
  return isFileExist(path) && !ext.isEmpty() && isStrInCandidate(ext);
}

bool ValueChecker::isIntInRange(const int v) const {
  return minV <= v && v < maxV;
}

bool ValueChecker::isSwitchString(const QString& switchs) const {
  return switchs.size() >= m_switchMinCnt && QSet<QChar>(switchs.cbegin(), switchs.cend()) == m_switchStates;
}

bool ValueChecker::operator()(const QVariant& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
      return isSpecifiedExtensionFileExist(v.toString());
    case CANDIDATE_STRING:
      return isStrInCandidate(v.toString());
    case FILE_PATH:
      return isFileExist(v.toString());
    case FOLDER_PATH:
      return isFolderExist(v.toString());
    case RANGE_INT:
      return isIntInRange(v.toInt());
    case SWITCH_STRING:
      return isSwitchString(v.toString());
    case PLAIN_STR:
    case PLAIN_INT:
    case PLAIN_BOOL:
    case PLAIN_FLOAT:
    case PLAIN_DOUBLE:
    case QSTRING_LIST:
    default:
      return true;
  }
}

QString ValueChecker::valueToString(const QVariant& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
    case CANDIDATE_STRING:
    case FILE_PATH:
    case FOLDER_PATH:
    case SWITCH_STRING:
    case PLAIN_STR:
      return v.toString();
    case RANGE_INT:
    case PLAIN_INT:
    case PLAIN_BOOL:
      return QString::number(v.toInt());
    case PLAIN_FLOAT:
      return QString::number(v.toFloat());
    case PLAIN_DOUBLE:
      return QString::number(v.toDouble());
    case QSTRING_LIST:
      return v.toStringList().join('\n');
    default:
      return "";
  }
}

QVariant ValueChecker::strToQVariant(const QString& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
    case CANDIDATE_STRING:
    case FILE_PATH:
    case FOLDER_PATH:
    case SWITCH_STRING:
    case PLAIN_STR:
      return v;
    case RANGE_INT:
    case PLAIN_INT:
      return v.toInt();
    case PLAIN_BOOL:
      return v == "true";
    case PLAIN_FLOAT:
      return v.toFloat();
    case PLAIN_DOUBLE:
      return v.toDouble();
    case QSTRING_LIST:
      return v.split('\n');
    default:
      return QVariant();
  }
}

KV::KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_) : name(name_), v(v_), checker(checker_) {}

QString KV::valueToString() const {
  return checker.valueToString(v);
}

QString KV::valueToString(const QVariant& v_) const {
  return checker.valueToString(v_);
}

const KV MemoryKey::DEFAULT_OPEN_PATH{"DEFAULT_OPEN_PATH", "./", ValueChecker{FOLDER_PATH}};
const KV MemoryKey::LANGUAGE_ZH_CN("LANGUAGE_ZH_CN", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::BACKGROUND_IMAGE("BACKGROUND_IMAGE", "", ValueChecker{{".png", ".webp", ".jpg", ".jpeg"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::SHOW_BACKGOUND_IMAGE("SHOW_BACKGOUND_IMAGE", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::PATH_LAST_TIME_COPY_TO("PATH_LAST_TIME_COPY_TO", "", ValueChecker{FOLDER_PATH});
const KV MemoryKey::PATH_JSON_EDITOR_LOAD_FROM("PATH_JSON_EDITOR_LOAD_FROM", "", ValueChecker{FOLDER_PATH});
const KV MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH("PATH_VIDEO_PLAYER_OPEN_PATH", "./", ValueChecker{FOLDER_PATH});
const KV MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE("PATH_PERFORMER_IMAGEHOST_LOCATE", "./", ValueChecker{FOLDER_PATH});
const KV MemoryKey::PATH_DB_INSERT_VIDS_FROM("PATH_DB_INSERT_VIDS_FROM", "./", ValueChecker{FOLDER_PATH});
const KV MemoryKey::PATH_DB_INSERT_TORRENTS_FROM("PATH_DB_INSERT_VIDS_FROM", "./", ValueChecker{FOLDER_PATH});

const KV MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW{"KEEP_VIDEOS_PLAYLIST_SHOW", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::AUTO_PLAY_NEXT_VIDEO("AUTO_PLAY_NEXT_VIDEO", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::SHOW_FOLDER_PREVIEW_HTML("SHOW_FOLDER_PREVIEW_HTML", true, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::FOLDER_PREVIEW_TYPE{"FOLDER_PREVIEW_TYPE", "labels", ValueChecker{{"3-lists", "browser", "lists"}, CANDIDATE_STRING}};
const KV MemoryKey::SHOW_FLOATING_PREVIEW{"SHOW_FLOATING_PREVIEW", true, ValueChecker{PLAIN_BOOL}};

const KV MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR("SHOW_QUICK_NAVIGATION_TOOL_BAR", true, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::SHOW_FRAMELESS_WINDOW("SHOW_FRAMELESS_WINDOW", true, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR("EXPAND_OFFICE_STYLE_MENUBAR", true, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA("QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA", false, ValueChecker{PLAIN_BOOL});

const KV MemoryKey::ITEM_VIEW_FONT_SIZE("ITEM_VIEW_FONT_SIZE", 12, ValueChecker{8, 25 + 1});
const KV MemoryKey::DEFAULT_VIDEO_PLAYER("DEFAULT_VIDEO_PLAYER", "Play", ValueChecker{{"Play in embedded player", "Play"}, CANDIDATE_STRING});
const KV MemoryKey::DEFAULT_NEW_CHOICE("DEFAULT_NEW_CHOICE", "New folder", ValueChecker{{"New folder", "New text", "New json", "New folder html"}, CANDIDATE_STRING});
const KV MemoryKey::DEFAULT_COPY_CHOICE("DEFAULT_COPY_CHOICE", "Copy fullpath", ValueChecker{{"Copy fullpath", "Copy path", "Copy name", "Copy the path"}, CANDIDATE_STRING});
const KV MemoryKey::DEFAULT_RENAME_CHOICE("DEFAULT_RENAME_CHOICE",
                                          "Rename (ith)",
                                          ValueChecker{{"Rename (ith)", "swap 1-2-3 to 1-3-2", "Case", "Str Inserter", "Str Deleter", "Str Replacer"}, CANDIDATE_STRING});
const KV MemoryKey::DEFAULT_EXTRACT_CHOICE("DEFAULT_EXTRACT_CHOICE",
                                           "Extract [0, 2) imgs",
                                           ValueChecker{{"Thumbnail Extractor", "Extract [0, 2) imgs", "Extract [0, 4) imgs", "Extract [b, e) imgs"}, CANDIDATE_STRING});

const KV MemoryKey::MOVE_TO_PATH_HISTORY("MOVE_TO_PATH_HISTORY", ".\n..\n\\", ValueChecker{PLAIN_STR});
const KV MemoryKey::COPY_TO_PATH_HISTORY("COPY_TO_PATH_HISTORY", ".\n..\n\\", ValueChecker{PLAIN_STR});
const KV MemoryKey::WHERE_CLAUSE_HISTORY("WHERE_CLAUSE_HISTORY", "A\nA&B\nA|B", ValueChecker{PLAIN_STR});

const KV MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX("MENU_RIBBON_CURRENT_TAB_INDEX", 0, ValueChecker{0});
const KV MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT("COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT", 2, ValueChecker{0});

const KV MemoryKey::TABLE_DEFAULT_SECTION_SIZE("TABLE_DEFAULT_SECTION_SIZE", 24, ValueChecker{0});
const KV MemoryKey::TABLE_DEFAULT_COLUMN_SECTION_SIZE("TABLE_DEFAULT_COLUMN_SECTION_SIZE", 200, ValueChecker{0});
const KV MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT("PERFORMER_IMAGE_FIXED_HEIGHT", 200, ValueChecker{0});

const KV MemoryKey::VIDS_LAST_TABLE_NAME("VIDS_LAST_TABLE_NAME", "", ValueChecker{PLAIN_STR});
const KV MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE("RIGHT_CLICK_TOOLBUTTON_STYLE", 0, ValueChecker{0, 4 + 1});

const KV MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES("NAME_PATTERN_USED_CREATE_BATCH_FILES", "Page %03d%1$1$11.html", ValueChecker{PLAIN_STR});
const KV MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS("NAME_PATTERN_USED_CREATE_BATCH_FOLDERS", "Page %03d%1$1$11", ValueChecker{PLAIN_STR});

const KV MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE("DIR_FILTER_ON_SWITCH_ENABLE", int(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::NoDotAndDotDot), ValueChecker{0});

const KV MemoryKey::VIDEO_PLAYER_VOLUME("VIDEO_PLAYER_VOLUME", 100, ValueChecker{0, 100 + 1});
const KV MemoryKey::VIDEO_PLAYER_MUTE("VIDEO_PLAYER_MUTE", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES("SEARCH_INCLUDING_SUBDIRECTORIES", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::SEARCH_NAME_CASE_SENSITIVE{"SEARCH_NAME_CASE_SENSITIVE", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE{"SEARCH_CONTENTS_CASE_SENSITIVE", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE("ADVANCE_SEARCH_LINEEDIT_VALUE", "", ValueChecker{PLAIN_STR});
const KV MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER{"DISABLE_ENTRIES_DONT_PASS_FILTER", true, ValueChecker{PLAIN_BOOL}};

const KV MemoryKey::RENAMER_INCLUDING_FILE_EXTENSION{"RENAMER_INCLUDING_FILE_EXTENSION", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::RENAMER_INCLUDING_DIR{"RENAMER_INCLUDING_DIR", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::RENAMER_OLD_STR_LIST{"RENAMER_OLD_STR_LIST",                                                                           //
                                         QStringList{"", "^(.*?)$", " BB ", " BB",                                                                        //
                                                     " - 360p", " - 480p", " - 516p", " - 720p", " - 1080p", " - 4K", " - FHD", " - UHD",  //
                                                     "([A-Z])", "( s\\d{1,2})"},                                                    //
                                                                                ValueChecker{QSTRING_LIST}}; //
const KV MemoryKey::RENAMER_NEW_STR_LIST{"RENAMER_NEW_STR_LIST",        //
                                         QStringList{"", " ", " \\1", "\\1 -", " - 1080p"},  //
                                                                                ValueChecker{QSTRING_LIST}};
const KV MemoryKey::RENAMER_INSERT_INDEXES_LIST{"RENAMER_INSERT_INDEXES_LIST", QStringList{"0", "50", "100", "128", "200"}, ValueChecker{QSTRING_LIST}};
const KV MemoryKey::RENAMER_ARRANGE_SECTION_INDEX{"RENAMER_ARRANGE_SECTION_INDEX", "1,2", ValueChecker{PLAIN_STR}};
const KV MemoryKey::RENAMER_REGEX_ENABLED{"RENAMER_REGEX_ENABLED", false, ValueChecker{PLAIN_BOOL}};

const KV MemoryKey::RENAMER_NUMERIAZER_START_INDEX{"RENAMER_NUMERIAZER_START_INDEX", 0, ValueChecker{INT32_MIN, INT32_MAX}};
const KV MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT{"RENAMER_NUMERIAZER_NO_FORMAT", QStringList{" %1", " - %1", " (%1)"}, ValueChecker{QSTRING_LIST}};
const KV MemoryKey::RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX{"RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX", 0, ValueChecker{0, 16}};

const KV MemoryKey::WIN32_MEDIAINFO_LIB_PATH("MEDIAINFO_LIB_PATH", "../lib/MediaInfo.lib", ValueChecker{{".lib", ".dll"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::WIN32_PERFORMERS_TABLE("PERFORMERS_TABLE", "../bin/PERFORMERS_TABLE.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::WIN32_AKA_PERFORMERS("AKA_PERFORMERS", "../bin/AKA_PERFORMERS.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::WIN32_STANDARD_STUDIO_NAME("STANDARD_STUDIO_NAME", "../bin/STANDARD_STUDIO_NAME.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH("TERMINAL_OPEN_BATCH_FILE_PATH", "../bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat", ValueChecker{{".bat"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::LOG_DEVEL_DEBUG{"LOG_DEVEL_DEBUG", false, ValueChecker{PLAIN_BOOL}};
const KV MemoryKey::WIN32_RUNLOG("RUNLOG", "../bin/RUNLOG", ValueChecker{FOLDER_PATH});
const KV MemoryKey::WIN32_RUND_IMG_PATH("RUND_IMG_PATH", ".", ValueChecker{FOLDER_PATH});

const KV MemoryKey::LINUX_MEDIAINFO_LIB_PATH("MEDIAINFO_LIB_PATH", "../lib/MediaInfo.dll", ValueChecker{{".dll"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::LINUX_PERFORMERS_TABLE("PERFORMERS_TABLE", "../bin/PERFORMERS_TABLE.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::LINUX_AKA_PERFORMERS("AKA_PERFORMERS", "../bin/AKA_PERFORMERS.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::LINUX_STANDARD_STUDIO_NAME("STANDARD_STUDIO_NAME", "../bin/STANDARD_STUDIO_NAME.txt", ValueChecker{{".txt"}, EXT_SPECIFIED_FILE_PATH});
const KV MemoryKey::LINUX_RUNLOG("RUNLOG", "../bin/RUNLOG", ValueChecker{FOLDER_PATH});
const KV MemoryKey::LINUX_RUND_IMG_PATH("RUND_IMG_PATH", ".", ValueChecker{FOLDER_PATH});

const KV MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION("DUPLICATE_FINDER_DEVIATION_DURATION", 2 * 1000, ValueChecker{0, 20 * 1000});         // 2s ~ 20s
const KV MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE("DUPLICATE_FINDER_DEVIATION_FILESIZE", 2 * 1024, ValueChecker{0, 30 * 1024 * 1024});  // 2kB ~ 30MB

const KV MemoryKey::SHOW_HAR_IMAGE_PREVIEW{"SHOW_HAR_IMAGE_PREVIEW", false, ValueChecker{PLAIN_BOOL}};
