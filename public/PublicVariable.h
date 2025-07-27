#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QString>
#include <QStringList>
#include <functional>

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;
typedef std::function<void()> T_SwitchStackWidget;

struct SystemPath {
  static const QString HOME_PATH;
  static const QString STARRED_PATH;
  static const QString VIDS_DATABASE;
  static const QString DEVICES_AND_DRIVES_DATABASE;
  static const QString AI_MEDIA_DUP_DATABASE;
  static const QString RECYCLE_BIN_DATABASE;
  static const QString PEFORMERS_DATABASE;
  static const QString TORRENTS_DATABASE;
  static const QString PRODUCTION_STUDIOS_DATABASE;
};

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

#include <QRegularExpression>
namespace JSON_RENAME_REGEX {
const QRegularExpression INVALID_CHARS_IN_FILENAME{R"([#\\/:*?<>|])"};  // #\/:*?<>|
const QRegularExpression INVALID_QUOTE_IN_FILENAME{R"([’“”"])"};        //'

const QRegularExpression TORRENT_LEADING_STR_COMP{R"(^((\[GT\])|(\[FFL\])|(\[FL\])))",
                                                  QRegularExpression::PatternOption::CaseInsensitiveOption};  // delete it
const QRegularExpression LEADING_OPEN_BRACKET_COMP{R"(^[\[\{\(])"};                                            //-

const QRegularExpression CONTINOUS_SPACE_COMP{R"(\s\s+)"};  //' '

const QRegularExpression NON_LEADING_BRACKET_COMP{R"([\(\{\[\)\}\]–])"};  //-
const QRegularExpression SPACE_HYPEN_SPACE_COMP{R"(\s*-\s*)"};            //-
const QRegularExpression CONTINOUS_HYPEN_COMP{"--+"};                     //-

const QRegularExpression HYPEN_OR_SPACE_END_WITH_DOT_COMP{R"([\s-]\.)"};  //.
const QRegularExpression TRAILING_HYPEN_COMP{"-$"};                       // delete it

const QRegularExpression DISCRAD_LETTER_COMP{R"([^A-Zãáéíóúüñ¿¡0-9_@# '])", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression INVALID_TABLE_NAME_LETTER{R"([^A-Z0-9_])", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression INVALID_GOOGLE_SEARCH_LETTER{R"([^A-Z0-9])", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression AND_COMP{R"( and | fucked by | fucked | fucks | fuck |/+|\+)", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression RESOLUTION_COMP{R"(2160p|1080p|360p|480p|720p|810p|4K|FHD|HD|SD)", QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression AT_LEAST_1_SPACE_COMP{R"(\s+)"};

// 正则1：匹配小写/数字后的大写字母
// 正则2：匹配小写字母后的数字
const QRegularExpression SPLIT_BY_UPPERCASE_COMP1("([a-z0-9])([A-Z])");
const QRegularExpression SPLIT_BY_UPPERCASE_COMP2("([[:alpha:]])([0-9])");
}  // namespace JSON_RENAME_REGEX

#endif  // PUBLICVARIABLE_H
