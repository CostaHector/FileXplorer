#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonValue>
#include <QRect>
#include <QSettings>
#include <QSize>
#include <QTextStream>

const QRect DEFAULT_GEOMETRY(0, 0, 1024, 768);
const QSize DOCKER_DEFAULT_SIZE(DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height());
const QString PROJECT_PATH{"../FileExplorerReadOnly"};

static inline QSettings& PreferenceSettings() {
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", "FileExplorerReadOnly");
  return settings;
}

auto TextReader(const QString& textPath) -> QString;

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;
typedef std::function<void()> T_SwitchStackWidget;

namespace MainKey {
constexpr int Name = 0;
constexpr int Size = 1;
constexpr int Type = 2;
constexpr int DateModified = 3;
const QStringList EXPLORER_COLUMNS_TITLE{"Name", "Size", "Type", "DateModified"};

const int NAME_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Name");
const int TYPE_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Type");
const int EXPLORER_COLUMNS_COUNT = EXPLORER_COLUMNS_TITLE.size();
}  // namespace MainKey

namespace DB_HEADER_KEY {
const QString Name = "Name";
const QString Size = "Size";
const QString Type = "Type";
const QString DateModified = "DateModified";

const QString Performers = "Performers";
const QString Tags = "Tags";
const QString Rate = "Rate";
const QString Driver = "Driver";
const QString Prepath = "Prepath";
const QString Extra = "Extra";

const QString ForSearch = "ForSearch";

const QStringList DB_HEADER{Name, Size, Type, DateModified, Performers, Tags, Rate, Driver, Prepath, Extra, ForSearch};
const int DB_DRIVER_INDEX = DB_HEADER.indexOf(Driver);
const int DB_PREPATH_INDEX = DB_HEADER.indexOf(Prepath);
const int DB_NAME_INDEX = DB_HEADER.indexOf(Name);
const int DB_SIZE_COLUMN = DB_HEADER.indexOf(Size);
const int DB_TYPE_INDEX = DB_HEADER.indexOf(Type);
const int DB_FOR_SEARCH_INDEX = DB_HEADER.indexOf(ForSearch);
}  // namespace DB_HEADER_KEY

namespace SearchKey {
const QString Name = "Name";
const QString Size = "Size";
const QString Type = "Type";
const QString DateModified = "DateModified";
const QString RelPath = "RelPath";
const QStringList EXPLORER_COLUMNS_TITLE{Name, Size, Type, DateModified, RelPath};
const int NAME_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf(Name);
const int TYPE_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf(Type);
const int RELPATH_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf(RelPath);
const int EXPLORER_COLUMNS_COUNT = EXPLORER_COLUMNS_TITLE.size();
}  // namespace SearchKey

namespace HEADERVIEW_SORT_INDICATOR_ORDER {
class OrderClass {
 public:
  QString name;
  Qt::SortOrder value;
};
const OrderClass AscendingOrder{"AscendingOrder", Qt::SortOrder::AscendingOrder};
const OrderClass DescendingOrder{"DescendingOrder", Qt::SortOrder::DescendingOrder};
const QMap<QString, Qt::SortOrder> string2SortOrderEnumListTable = {{AscendingOrder.name, AscendingOrder.value}, {DescendingOrder.name, DescendingOrder.value}};
const QStringList HEADERVIEW_SORT_INDICATOR_ORDER_STR = string2SortOrderEnumListTable.keys();
QString SortOrderEnum2String(const Qt::SortOrder orderEnum);
}  // namespace HEADERVIEW_SORT_INDICATOR_ORDER

class ValueChecker {
 public:
  friend struct KV;

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

  explicit ValueChecker(const QStringList& candidates, const VALUE_TYPE valueType_ = EXT_SPECIFIED_FILE_PATH);

  explicit ValueChecker(int minV_ = INT32_MIN, int maxV_ = INT32_MAX);
  explicit ValueChecker(const QSet<QChar>& chars = {'0', '1'}, int minLength = 1);

  explicit ValueChecker(const VALUE_TYPE valueType_);

  static int getFileExtensionDotIndex(const QString& path);
  static QString GetFileExtension(const QString& path);  // e.g. .txt .bat

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
  VALUE_TYPE valueType;

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

constexpr char MOVE_COPT_TO_PATH_STR_SEPERATOR = '\n';

namespace MemoryKey {
const KV DEFAULT_OPEN_PATH{"DEFAULT_OPEN_PATH", "./", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH}};
const KV LANGUAGE_ZH_CN("LANGUAGE_ZH_CN", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV BACKGROUND_IMAGE("BACKGROUND_IMAGE", "", ValueChecker{{".png", ".webp", ".jpg", ".jpeg"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV SHOW_BACKGOUND_IMAGE("SHOW_BACKGOUND_IMAGE", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV PATH_LAST_TIME_COPY_TO("PATH_LAST_TIME_COPY_TO", "", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV PATH_JSON_EDITOR_LOAD_FROM("PATH_JSON_EDITOR_LOAD_FROM", "", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV PATH_VIDEO_PLAYER_OPEN_PATH("PATH_VIDEO_PLAYER_OPEN_PATH", "./", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV PATH_PERFORMER_IMAGEHOST_LOCATE("PATH_PERFORMER_IMAGEHOST_LOCATE", "./", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV PATH_DB_INSERT_VIDS_FROM("PATH_DB_INSERT_VIDS_FROM", "./", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV PATH_DB_INSERT_TORRENTS_FROM("PATH_DB_INSERT_VIDS_FROM", "./", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});

const KV KEEP_VIDEOS_PLAYLIST_SHOW{"KEEP_VIDEOS_PLAYLIST_SHOW", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV AUTO_PLAY_NEXT_VIDEO("AUTO_PLAY_NEXT_VIDEO", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV SHOW_FOLDER_PREVIEW_HTML("SHOW_FOLDER_PREVIEW_HTML", true, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV FOLDER_PREVIEW_TYPE{"FOLDER_PREVIEW_TYPE", "labels", ValueChecker{{"labels", "browser", "lists"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING}};

const KV SHOW_QUICK_NAVIGATION_TOOL_BAR("SHOW_QUICK_NAVIGATION_TOOL_BAR", true, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV SHOW_FRAMELESS_WINDOW("SHOW_FRAMELESS_WINDOW", true, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV EXPAND_OFFICE_STYLE_MENUBAR("EXPAND_OFFICE_STYLE_MENUBAR", true, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA("QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});

const KV HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX("HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX", MainKey::Name, ValueChecker{0});

const KV HEADVIEW_SORT_INDICATOR_ORDER("HEADVIEW_SORT_INDICATOR_ORDER",
                                       HEADERVIEW_SORT_INDICATOR_ORDER::AscendingOrder.name,
                                       ValueChecker{HEADERVIEW_SORT_INDICATOR_ORDER::HEADERVIEW_SORT_INDICATOR_ORDER_STR, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV ITEM_VIEW_FONT_SIZE("ITEM_VIEW_FONT_SIZE", 12, ValueChecker{8, 25 + 1});
const KV DEFAULT_VIDEO_PLAYER("DEFAULT_VIDEO_PLAYER", "Play", ValueChecker{{"Play in embedded player", "Play"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV DEFAULT_NEW_CHOICE("DEFAULT_NEW_CHOICE", "New folder", ValueChecker{{"New folder", "New text", "New json", "New folder html"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV DEFAULT_COPY_CHOICE("DEFAULT_COPY_CHOICE", "Copy fullpath", ValueChecker{{"Copy fullpath", "Copy path", "Copy name", "Copy the path"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV DEFAULT_RENAME_CHOICE("DEFAULT_RENAME_CHOICE",
                               "Rename (ith)",
                               ValueChecker{{"Rename (ith)", "swap 1-2-3 to 1-3-2", "Case", "Str Inserter", "Str Deleter", "Str Replacer"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV MOVE_TO_PATH_HISTORY("MOVE_TO_PATH_HISTORY", ".\n..\n\\", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});
const KV COPY_TO_PATH_HISTORY("COPY_TO_PATH_HISTORY", ".\n..\n\\", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});
const KV WHERE_CLAUSE_HISTORY("WHERE_CLAUSE_HISTORY", "A\nA&B\nA|B", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});

const KV MENU_RIBBON_CURRENT_TAB_INDEX("MENU_RIBBON_CURRENT_TAB_INDEX", 0, ValueChecker{0});
const KV COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT("COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT", 2, ValueChecker{0});

const KV TABLE_DEFAULT_SECTION_SIZE("TABLE_DEFAULT_SECTION_SIZE", 24, ValueChecker{0});
const KV TABLE_DEFAULT_COLUMN_SECTION_SIZE("TABLE_DEFAULT_COLUMN_SECTION_SIZE", 200, ValueChecker{0});
const KV PERFORMER_IMAGE_FIXED_HEIGHT("PERFORMER_IMAGE_FIXED_HEIGHT", 200, ValueChecker{0});

const KV VIDS_LAST_TABLE_NAME("VIDS_LAST_TABLE_NAME", "", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});
const KV RIGHT_CLICK_TOOLBUTTON_STYLE("RIGHT_CLICK_TOOLBUTTON_STYLE", 0, ValueChecker{0, 4 + 1});

const KV NAME_PATTERN_USED_CREATE_BATCH_FILES("NAME_PATTERN_USED_CREATE_BATCH_FILES", "Page %03d%1$1$11.html", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});
const KV NAME_PATTERN_USED_CREATE_BATCH_FOLDERS("NAME_PATTERN_USED_CREATE_BATCH_FOLDERS", "Page %03d%1$1$11", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});

const KV DIR_FILTER_ON_SWITCH_ENABLE("DIR_FILTER_ON_SWITCH_ENABLE", int(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::NoDotAndDotDot), ValueChecker{0});

const KV VIDEO_PLAYER_VOLUME("VIDEO_PLAYER_VOLUME", 100, ValueChecker{0, 100 + 1});
const KV VIDEO_PLAYER_MUTE("VIDEO_PLAYER_MUTE", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV SEARCH_INCLUDING_SUBDIRECTORIES("SEARCH_INCLUDING_SUBDIRECTORIES", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL});
const KV SEARCH_MODE_DEFAULT_VALUE("SEARCH_MODE_DEFAULT_VALUE", "Normal", ValueChecker{{"Normal", "Wildcard", "Regex", "Search for File Content"}, ValueChecker::VALUE_TYPE::CANDIDATE_STRING});
const KV SEARCH_NAME_CASE_SENSITIVE{"SEARCH_NAME_CASE_SENSITIVE", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV SEARCH_CONTENTS_CASE_SENSITIVE{"SEARCH_CONTENTS_CASE_SENSITIVE", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV ADVANCE_SEARCH_LINEEDIT_VALUE("ADVANCE_SEARCH_LINEEDIT_VALUE", "", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR});
const KV DISABLE_ENTRIES_DONT_PASS_FILTER{"DISABLE_ENTRIES_DONT_PASS_FILTER", true, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};

const KV RENAMER_INCLUDING_FILE_EXTENSION{"RENAMER_INCLUDING_FILE_EXTENSION", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV RENAMER_INCLUDING_DIR{"RENAMER_INCLUDING_DIR", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV RENAMER_OLD_STR_LIST{"RENAMER_OLD_STR_LIST", QStringList{"", " BB ", " BB", " - 360p", " - 480p", " - 516p", " - 720p", " - 1080p", " - 4K", " - FHD", " - UHD"},
                              ValueChecker{ValueChecker::VALUE_TYPE::QSTRING_LIST}};
const KV RENAMER_NEW_STR_LIST{"RENAMER_NEW_STR_LIST", QStringList{"", " ", "", " - 1080p"}, ValueChecker{ValueChecker::VALUE_TYPE::QSTRING_LIST}};
const KV RENAMER_INSERT_INDEXES_LIST{"RENAMER_INSERT_INDEXES_LIST", QStringList{"0", "50", "100", "128", "200"}, ValueChecker{ValueChecker::VALUE_TYPE::QSTRING_LIST}};
const KV RENAMER_ARRANGE_SECTION_INDEX{"RENAMER_ARRANGE_SECTION_INDEX", "1,2", ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_STR}};

const KV RENAMER_NUMERIAZER_START_INDEX{"RENAMER_NUMERIAZER_START_INDEX", 0, ValueChecker{INT32_MIN, INT32_MAX}};
const KV RENAMER_NUMERIAZER_NO_FORMAT{"RENAMER_NUMERIAZER_NO_FORMAT", QStringList{" %1", " - %1", " (%1)"}, ValueChecker{ValueChecker::VALUE_TYPE::QSTRING_LIST}};
const KV RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX{"RENAMER_NUMERIAZER_NO_FORMAT_DEFAULT_INDEX", 0, ValueChecker{0, 16}};

const KV WIN32_MEDIAINFO_LIB_PATH("WIN32_MEDIAINFO_LIB_PATH", "../bin/lib/MediaInfo.dll", ValueChecker{{".dll"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV WIN32_PERFORMERS_TABLE("WIN32_PERFORMERS_TABLE", "../bin/PERFORMERS_TABLE.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV WIN32_AKA_PERFORMERS("WIN32_AKA_PERFORMERS", "../bin/AKA_PERFORMERS.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV WIN32_STANDARD_STUDIO_NAME("WIN32_STANDARD_STUDIO_NAME", "../bin/STANDARD_STUDIO_NAME.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV WIN32_TERMINAL_OPEN_BATCH_FILE_PATH("WIN32_TERMINAL_OPEN_BATCH_FILE_PATH",
                                             "../bin/WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.bat",
                                             ValueChecker{{".bat"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV LOG_DEVEL_DEBUG{"LOG_DEVEL_DEBUG", false, ValueChecker{ValueChecker::VALUE_TYPE::PLAIN_BOOL}};
const KV WIN32_RUNLOG("WIN32_RUNLOG", "../bin/RUNLOG", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV WIN32_RUND_IMG_PATH("WIN32_RUND_IMG_PATH", ".", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});

const KV LINUX_MEDIAINFO_LIB_PATH("LINUX_MEDIAINFO_LIB_PATH", "../bin/lib/MediaInfo.dll", ValueChecker{{".dll"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});

const KV LINUX_PERFORMERS_TABLE("LINUX_PERFORMERS_TABLE", "../bin/PERFORMERS_TABLE.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV LINUX_AKA_PERFORMERS("LINUX_AKA_PERFORMERS", "../bin/AKA_PERFORMERS.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV LINUX_STANDARD_STUDIO_NAME("LINUX_STANDARD_STUDIO_NAME", "../bin/STANDARD_STUDIO_NAME.txt", ValueChecker{{".txt"}, ValueChecker::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH});
const KV LINUX_RUNLOG("LINUX_RUNLOG", "../bin/RUNLOG", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});
const KV LINUX_RUND_IMG_PATH("LINUX_RUND_IMG_PATH", ".", ValueChecker{ValueChecker::VALUE_TYPE::FOLDER_PATH});

const KV DUPLICATE_FINDER_DEVIATION_DURATION("DUPLICATE_FINDER_DEVIATION_DURATION", 2 * 1000, ValueChecker{0, 20 * 1000});         // 2s ~ 20s
const KV DUPLICATE_FINDER_DEVIATION_FILESIZE("DUPLICATE_FINDER_DEVIATION_FILESIZE", 2 * 1024, ValueChecker{0, 30 * 1024 * 1024});  // 2kB ~ 30MB
}  // namespace MemoryKey

namespace SystemPath {
const QString drivePath = "";
const QString desktopPath = QDir(QDir::homePath()).absoluteFilePath("Desktop");
const QString documentPath = QDir(QDir::homePath()).absoluteFilePath("Documents");
const QString downloadPath = QDir(QDir::homePath()).absoluteFilePath("Downloads");
const QString musicPath = QDir(QDir::homePath()).absoluteFilePath("Music");
const QString picturesPath = QDir(QDir::homePath()).absoluteFilePath("Pictures");
const QString videosPath = QDir(QDir::homePath()).absoluteFilePath("Videos");
const QString starredPath = QDir(QDir::homePath()).absoluteFilePath("Documents");
const QString VIDS_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/VIDS_DATABASE.db");
const QString AI_MEDIA_DUP_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/DUPLICATES_DB.db");
const QString RECYCLE_BIN_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/RECYCLE_BIN_DATABASE.db");
const QString PEFORMERS_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/PERFORMERS_DATABASE.db");
const QString TORRENTS_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/TORRENTS_DATABASE.db");
const QString PRODUCTION_STUDIOS_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FileExplorerReadOnly/PRODUCTION_STUDIOS_DATABASE.db");
}  // namespace SystemPath

namespace DB_TABLE {
const QString MOVIES = "MOVIES";
const QString PERFORMERS = "PERFORMERS";
const QString TORRENTS = "TORRENTS";
}  // namespace DB_TABLE

extern const char* SUBMIT_BTN_STYLE;

constexpr int TABS_ICON_IN_MENU_3x1 = 16;
constexpr int TABS_ICON_IN_MENU_2x1 = 24;
constexpr int TABS_ICON_IN_MENU_1x1 = 48;

namespace TYPE_FILTER {
const QStringList AI_DUP_VIDEO_TYPE_SET = {"*.mp4", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts"};
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts", "*.vob", "*.iso"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat"};
const QStringList BUILTIN_COMPRESSED_TYPE_SET = {"*.qz"};
}  // namespace TYPE_FILTER

enum class CCMMode { ERROR_OP = -1, MERGE_OP = 0, COPY_OP = 1, CUT_OP = 2, LINK_OP = 3 };

static const QMap<CCMMode, QString> CCMMode2QString = {{CCMMode::MERGE_OP, "MERGE"}, {CCMMode::COPY_OP, "COPY"}, {CCMMode::CUT_OP, "CUT"}, {CCMMode::LINK_OP, "LINK"}};

#include <QColor>
#include <QRegularExpression>

namespace JSON_RENAME_REGEX {
const QRegularExpression invalidCharPat("[\\#\\\\/\\:\\*\\?\\<\\>\\|]");  // #\/:*?<>
const QRegularExpression invalidQuotePat("[’“”\"]");                      //'

const QRegularExpression leadingStrComp("^((\\[FL\\])|(\\[FFL\\])|(\\[GT\\]))", QRegularExpression::PatternOption::CaseInsensitiveOption);  // delete it
const QRegularExpression leadingOpenBracketComp("^[\[\{\(]");                                                                               //-                                         //-

const QRegularExpression continuousSpaceComp("\\s\\s+");  //' '

const QRegularExpression nonLeadingBracketComp("[\\(\\{\\[\\)\\}\\]–]");  //-
const QRegularExpression spaceBarSpaceComp("\\s*-\\s*");                  //-
const QRegularExpression continousHypenComp("--+");                       //-

const QRegularExpression hypenOrSpaceFollowedWithDotPat("[\\s-]\\.");  //.
const QRegularExpression trailingHypenComp("-$");                      // delete it

const QRegularExpression DISCRAD_LETTER_COMP("[^A-Zãáéíóúüñ¿¡0-9._@# ']", QRegularExpression::PatternOption::CaseInsensitiveOption);
const QRegularExpression INVALID_TABLE_NAME_LETTER("[^A-Z0-9_]", QRegularExpression::PatternOption::CaseInsensitiveOption);
const QRegularExpression AND_COMP(" and | fucked by | fucked | fucks | fuck |\\+", QRegularExpression::PatternOption::CaseInsensitiveOption);
const QRegularExpression RESOLUTION_COMP("2160p|1080p|360p|480p|720p|810p|4K|FHD|HD|SD", QRegularExpression::PatternOption::CaseInsensitiveOption);
const QRegularExpression CONTINOUS_SPACE("\\s+");

const QRegularExpression SPLIT_BY_UPPERCASE("([A-Z0-9]\\d{0,4})", QRegularExpression::PatternOption::NoPatternOption);
}  // namespace JSON_RENAME_REGEX

namespace STATUS_COLOR {
const QColor LIGHT_GREEN_COLOR(245, 245, 220);
const QColor TOMATO_COLOR(244, 164, 96);
const QColor TRANSPARENT_COLOR(Qt::GlobalColor::color0);
}  // namespace STATUS_COLOR

bool VerifyOneFilePath(const KV& kv, const QString& fileType = "txt");
bool VerifyOneFolderPath(const KV& kv);
bool InitOutterPlainTextPath();

constexpr int CONTROL_TOOLBAR_HEIGHT = 28;
#endif  // PUBLICVARIABLE_H
