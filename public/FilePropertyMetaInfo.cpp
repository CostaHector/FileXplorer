#include "FilePropertyMetaInfo.h"
#include "PathTool.h"

namespace FilePropertyHelper {

QString FilePropertyInfo::GetAbsolutePath(const QString& rootPath) const {
  return PathTool::GetAbsFilePathFromRootRelName(rootPath, m_RelPath, ""); // endswith '/'
}

QString FilePropertyInfo::GetAbsoluteFilePath(const QString& rootPath) const {
  return PathTool::GetAbsFilePathFromRootRelName(rootPath, m_RelPath, m_Name);
}

}
