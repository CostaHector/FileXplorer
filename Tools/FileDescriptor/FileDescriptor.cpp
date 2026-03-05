#include "FileDescriptor.h"
#include "MD5Calculator.h"
#include "BytesRangeTool.h"

namespace FileDescriptor {
QByteArray GetFileUniquedId(const QString& fileAbsPath) {
  return MD5Calculator::GetFileMD5(fileAbsPath, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
}
QList<QByteArray> GetFileUniquedIds(const QStringList& files) {
  return MD5Calculator::GetBatchFileMD5(files, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
}
}
