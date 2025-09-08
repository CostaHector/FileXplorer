#ifndef JSONRENAMEREGEX_H
#define JSONRENAMEREGEX_H

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

#endif // JSONRENAMEREGEX_H
