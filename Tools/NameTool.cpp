#include "NameTool.h"

const char* NameTool::FIELD_SEPERATOR = "&|\\band\\b|,|\t|\n|@|#|\\+|\\||/|\\\\";
const QRegularExpression NameTool::FS_COMP(FIELD_SEPERATOR, QRegularExpression::PatternOption::CaseInsensitiveOption);

const char* NameTool::FREQUENT_NAME_PATTER = "[A-Z]{2,}\\s[A-Z']+(\\s[A-Z]{2,})*";
const QRegularExpression NameTool::NAME_COMP(FREQUENT_NAME_PATTER);

