#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QDir>
#include <QRect>
#include <QSettings>
#include <QSize>

constexpr QRect DEFAULT_GEOMETRY{0, 0, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height()};
const char SUBMIT_BTN_STYLE[] =
    "QPushButton{"
    "    color: #fff;"
    "    background-color: DodgerBlue;"
    "    border-color: DodgerBlue;"
    "}"
    "QPushButton:hover {"
    "    color: #fff;"
    "    background-color: rgb(36, 118, 199);"
    "    border-color: rgb(36, 118, 199);"
    "}";

struct IMAGE_SIZE {
  static constexpr int TABS_ICON_IN_MENU_16 = 16;
  static constexpr int TABS_ICON_IN_MENU_24 = 24;
  static constexpr int TABS_ICON_IN_MENU_48 = 48;
  static int IMG_WIDTH;
  static int IMG_HEIGHT;
};

static inline QSettings& PreferenceSettings() {
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", "FileExplorerReadOnly");
  return settings;
}

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;
typedef std::function<void()> T_SwitchStackWidget;

namespace SystemPath {
const QString HOME_PATH{QDir::homePath()};
const QString STARRED_PATH{HOME_PATH + "/Documents"};
const QString VIDS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/VIDS_DATABASE.db"};
const QString DEVICES_AND_DRIVES_DATABASE{HOME_PATH + "/FileExplorerReadOnly/DEVICES_AND_DRIVES.db"};
const QString AI_MEDIA_DUP_DATABASE{HOME_PATH + "/FileExplorerReadOnly/DUPLICATES_DB.db"};
const QString RECYCLE_BIN_DATABASE{HOME_PATH + "/FileExplorerReadOnly/RECYCLE_BIN_DATABASE.db"};
const QString PEFORMERS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/PERFORMERS_DATABASE.db"};
const QString TORRENTS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/TORRENTS_DATABASE.db"};
const QString PRODUCTION_STUDIOS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/PRODUCTION_STUDIOS_DATABASE.db"};
}  // namespace SystemPath

namespace DB_TABLE {
const QString MOVIES = "MOVIES";
const QString PERFORMERS = "PERFORMERS";
const QString TORRENTS = "TORRENTS";
const QString DISKS = "DISKS";
}  // namespace DB_TABLE

namespace TYPE_FILTER {
const QStringList AI_DUP_VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts"};
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts", "*.vob", "*.iso", "*.3gp"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat"};
const QStringList BUILTIN_COMPRESSED_TYPE_SET = {"*.qz"};
const QStringList HAR_TYPE_SET = {"*.har"};
}  // namespace TYPE_FILTER

class CCMMode {
 public:
  enum Mode {
    MERGE_OP = 0,  //
    COPY_OP = 1,   //
    CUT_OP = 2,    //
    LINK_OP = 3,   //
    ERROR_OP = 4,  //
    BUTT
  };
  static constexpr int MCCL2STR_MAX_LEN = 20;
  static const char MCCL2STR[BUTT][MCCL2STR_MAX_LEN];
};

#include <QRegularExpression>
namespace JSON_RENAME_REGEX {
const QRegularExpression INVALID_CHARS_IN_FILENAME{R"([#\\/:*?<>|])"};  // #\/:*?<>|
const QRegularExpression INVALID_QUOTE_IN_FILENAME(R"([’“”"])");        //'

const QRegularExpression TORRENT_LEADING_STR_COMP(R"(^((\[GT\])|(\[FFL\])|(\[FL\])))", QRegularExpression::PatternOption::CaseInsensitiveOption);  // delete it
const QRegularExpression LEADING_OPEN_BRACKET_COMP(R"(^[\[\{\(])");                                                                                //-                                         //-

const QRegularExpression CONTINOUS_SPACE_COMP{R"(\s\s+)"};  //' '

const QRegularExpression NON_LEADING_BRACKET_COMP{R"([\(\{\[\)\}\]–])"};  //-
const QRegularExpression SPACE_HYPEN_SPACE_COMP{R"(\s*-\s*)"};            //-
const QRegularExpression CONTINOUS_HYPEN_COMP{"--+"};                     //-

const QRegularExpression HYPEN_OR_SPACE_END_WITH_DOT_COMP{R"([\s-]\.)"};  //.
const QRegularExpression TRAILING_HYPEN_COMP{"-$"};                       // delete it

const QRegularExpression DISCRAD_LETTER_COMP{R"([^A-Zãáéíóúüñ¿¡0-9_@# '])", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression INVALID_TABLE_NAME_LETTER{R"([^A-Z0-9_])", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression AND_COMP{R"( and | fucked by | fucked | fucks | fuck |\+)", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression RESOLUTION_COMP{R"(2160p|1080p|360p|480p|720p|810p|4K|FHD|HD|SD)", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression AT_LEAST_1_SPACE_COMP{R"(\s+)"};

const QRegularExpression SPLIT_BY_UPPERCASE_COMP(R"((?<!^)([A-Z0-9]\d{0,4}))", QRegularExpression::PatternOption::NoPatternOption);

}  // namespace JSON_RENAME_REGEX

#endif  // PUBLICVARIABLE_H
