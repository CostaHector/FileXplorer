#include "VideoTestPrecoditionTools.h"
#include "VidDupTabFields.h"

namespace VideoTestPrecoditionTools {
SetDatabaseParmRetType setDupVidDbAbsFilePath(const QString& placeDbFileLocation) {
  QString actualDupDbAbsFilePath = VidDupHelper::DupVidDbAbsFilePathInTestCase(placeDbFileLocation);
  if (!actualDupDbAbsFilePath.startsWith(placeDbFileLocation, Qt::CaseSensitivity::CaseSensitive)) {
    return {false, actualDupDbAbsFilePath};
  }
  if (!actualDupDbAbsFilePath.endsWith(".db", Qt::CaseSensitivity::CaseSensitive)) {
    return {false, actualDupDbAbsFilePath};
  }
  return {true, actualDupDbAbsFilePath};
}
SetDatabaseParmRetType setDupVidDbConnectionName(const QString& newConnectionNameUsed, int lineNo) {
  const QString expect_connectionName = newConnectionNameUsed + QString::number(lineNo);
  const QString actualDupDbConnectionName = VidDupHelper::DupVidDbConnectionNameInTestCase(expect_connectionName);
  if (actualDupDbConnectionName != expect_connectionName) {
    return {false, actualDupDbConnectionName};
  }
  return {true, actualDupDbConnectionName};
}
} // namespace VideoTestPrecoditionTools
