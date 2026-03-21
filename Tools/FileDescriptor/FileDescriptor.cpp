#include "FileDescriptor.h"
#include "MD5Calculator.h"
#include "BytesRangeTool.h"
#include "FileTool.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include <QFile>

namespace FileDescriptor {
QByteArray GetFileUniquedId(const QString& fileAbsPath) {
  const QString& jsonPath{PathTool::FileExtReplacedWithJson(fileAbsPath)};
  if (QFile::exists(jsonPath)) {
    const QByteArray& md5{JsonHelper::GetMD5FromJsonFile(jsonPath)};
    if (!md5.isEmpty()) {
      return md5;
    }
  }
  return MD5Calculator::GetFileMD5(fileAbsPath, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
}

QList<QByteArray> GetFileUniquedIds(const QStringList& files) {
  return MD5Calculator::GetBatchFileMD5(files, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
}

}  // namespace FileDescriptor
