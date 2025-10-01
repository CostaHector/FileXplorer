#ifndef FILEPROPERTYMETAINFO_H
#define FILEPROPERTYMETAINFO_H

#include "PublicMacro.h"
#include <QDateTime>

#define SEARCH_OUT_FILE_INFO_KEY_MAPPING_MAIN                                               \
  SEARCH_OUT_FILE_INFO_KEY_ITEM(Name, 0, QString, DataFormatter::formatDefault)           \
  SEARCH_OUT_FILE_INFO_KEY_ITEM(Size, 1, qint64, DataFormatter::formatFileSizeGMKB)       \
  SEARCH_OUT_FILE_INFO_KEY_ITEM(Type, 2, QString, DataFormatter::formatDefault)           \
  SEARCH_OUT_FILE_INFO_KEY_ITEM(ModifiedDate, 3, QDateTime, DataFormatter::formatDefault) \
  SEARCH_OUT_FILE_INFO_KEY_ITEM(RelPath, 4, QString, DataFormatter::formatDefault)

// --

namespace FilePropertyHelper {
enum PropColumnE {
#define SEARCH_OUT_FILE_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) enu = enumVal,
  SEARCH_OUT_FILE_INFO_KEY_MAPPING_MAIN
#undef SEARCH_OUT_FILE_INFO_KEY_ITEM
};

constexpr const char* SEARCH_TABLE_HEADERS[]{
#define SEARCH_OUT_FILE_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) #enu,
    SEARCH_OUT_FILE_INFO_KEY_MAPPING_MAIN
#undef SEARCH_OUT_FILE_INFO_KEY_ITEM
};
constexpr int SEARCH_TABLE_HEADERS_COUNT = sizeof(SEARCH_TABLE_HEADERS) / sizeof(SEARCH_TABLE_HEADERS[0]);

struct FilePropertyInfo {
#define SEARCH_OUT_FILE_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) VariableType m_##enu;
  SEARCH_OUT_FILE_INFO_KEY_MAPPING_MAIN
#undef SEARCH_OUT_FILE_INFO_KEY_ITEM
  QString GetAbsolutePath(const QString& rootPath) const;
  QString GetAbsoluteFilePath(const QString& rootPath) const;
  bool operator==(const FilePropertyInfo& rhs) const { return m_Name == rhs.m_Name && m_RelPath == rhs.m_RelPath; }
  bool operator<(const FilePropertyInfo& rhs) const { return m_RelPath < rhs.m_RelPath || (m_RelPath == rhs.m_RelPath && m_Name < rhs.m_Name);}
};

using FilePropertyInfoList = QList<FilePropertyInfo>;
}  // namespace FilePropertyMetaInfo

#endif  // FILEPROPERTYMETAINFO_H
