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

constexpr QRect DEFAULT_GEOMETRY(0, 0, 1024, 768);
constexpr QSize DOCKER_DEFAULT_SIZE(DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height());
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
  static constexpr int TABS_ICON_IN_MENU_3x1 = 16;
  static constexpr int TABS_ICON_IN_MENU_2x1 = 24;
  static constexpr int TABS_ICON_IN_MENU_1x1 = 48;
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

namespace SystemPath {
const QString drivePath = "";
const QString desktopPath = QDir(QDir::homePath()).absoluteFilePath("Desktop");
const QString documentPath = QDir(QDir::homePath()).absoluteFilePath("Documents");
const QString downloadPath = QDir(QDir::homePath()).absoluteFilePath("Downloads");
const QString musicPath = QDir(QDir::homePath()).absoluteFilePath("Music");
const QString picturesPath = QDir(QDir::homePath()).absoluteFilePath("Pictures");
const QString videosPath = QDir(QDir::homePath()).absoluteFilePath("    Videos");
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

namespace TYPE_FILTER {
const QStringList AI_DUP_VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts"};
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts", "*.vob", "*.iso", "*.3gp"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat"};
const QStringList BUILTIN_COMPRESSED_TYPE_SET = {"*.qz"};
const QStringList HAR_TYPE_SET = {"*.har"};
}  // namespace TYPE_FILTER

enum class CCMMode { ERROR_OP = -1, MERGE_OP = 0, COPY_OP = 1, CUT_OP = 2, LINK_OP = 3 };
static const QMap<CCMMode, QString> CCMMode2QString = {{CCMMode::MERGE_OP, "MERGE"},  //
                                                       {CCMMode::COPY_OP, "COPY"},    //
                                                       {CCMMode::CUT_OP, "CUT"},      //
                                                       {CCMMode::LINK_OP, "LINK"}};

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

#endif  // PUBLICVARIABLE_H
