#ifndef BATCHRENAMEBY_H
#define BATCHRENAMEBY_H

#include <QStringList>

namespace BatchRenameBy {
enum class RnmResult {
  PARTIAL_FAILED = -1,
  SKIP = 0,
  ALL_SUCCEED = 1,
};

QStringList GetFilesNeedRename(const QString& path, const QStringList& jsonNames);
RnmResult InsertQueryAndConfirm(const QString& workPath,           //
                                const QStringList& selectedNames,  //
                                const QString& defStrInsert = "",  //
                                const int insertAtIndex = 0);
RnmResult ReplaceQueryAndConfirm(const QString& workPath,           //
                                 const QStringList& selectedNames,  //
                                 const QString& defOldName = "",    //
                                 const QString& defNewName = "",    //
                                 bool disableOldNameEdit = false);
RnmResult NumerizerQueryAndConfirm(const QString& workPath,  //
                                   const QStringList& selectedNames);
// jsonNames or videoName format can only be "coreName.ext"
// all file with name format "^coreName.ext" or "^coreName - \d{1,3}.ext", or "^coreName \d{1,3}.ext" will filtered out for rename
int InsertBySpecifiedJson(const QString& path, const QStringList& jsonNames);     // jsonNames can also be videoNames
int ReplaceBySpecifiedJson(const QString& path, const QStringList& jsonNames);    // jsonNames can also be videoNames
int NumerizerBySpecifiedJson(const QString& path, const QStringList& jsonNames);  // jsonNames can also be videoNames

// imgNames format can be "coreName.ext" or "coreName - \d{1,3}.ext" or "coreName \d{1,3}.ext"
// all file with name format "^coreName" will filtered out for rename
}  // namespace BatchRenameBy

#endif  // BATCHRENAMEBY_H
