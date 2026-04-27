#include "DuplicateVideosHelper.h"
#include "JsonRenameRegex.h"

QString GetTableName(const QString& pathName) {
  QString tableName{pathName};
  tableName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, "_");  // onlt support path with [0-9a-z_]
  return tableName;
}

QString TableName2Path(const QString& tableName) {
  QString ans{tableName};
#ifdef _WIN32
  if (ans.size() >= 3 && ans[1] == '_' && ans[2] == '_') {
    ans[1] = ':';
  }
#endif
  ans.replace('_', '/');
  return ans;
}

