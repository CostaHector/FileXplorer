#include "VidDupTabFields.h"
#include "PublicVariable.h"
#include "Logger.h"
#include <QFileInfo>
#include <QSet>
namespace VidDupHelper {

#ifdef RUNNING_UNIT_TESTS
QString& DupVidDbAbsFilePathInTestCase(const QString& aNewPrePath) { // empty string as input: keep the former value
  static QString vidDbPath;
  if (!aNewPrePath.isEmpty()) {
    if (QFileInfo(aNewPrePath).isDir()) {
      vidDbPath = aNewPrePath + "/DUPLICATES_VIDEOS_DB_TEST.db";
    } else {
      LOG_E("Path[%s] is not a directory", qPrintable(aNewPrePath));
    }
  }
  return vidDbPath;
}

QString& DupVidDbConnectionNameInTestCase(const QString& aNewConnectionName) { // empty string as input: keep the former value
  static QString connectionName;
  if (!aNewConnectionName.isEmpty()) {
    static QSet<QString> occupiedConnectionName;
    if (!occupiedConnectionName.contains(aNewConnectionName)) {
      connectionName = aNewConnectionName;
    } else {
      LOG_E("connection name[%s] occupied.", qPrintable(aNewConnectionName));
    }
  }
  return connectionName;
}

#endif

QString GetAiDupVidDbPath() {
#ifdef RUNNING_UNIT_TESTS
  return DupVidDbAbsFilePathInTestCase("");
#else
  return SystemPath::AI_MEDIA_DUP_DATABASE();
#endif
}

QString GetAiDupVidDbConnectionName() {
#ifdef RUNNING_UNIT_TESTS
  return DupVidDbConnectionNameInTestCase("");
#else
  static constexpr char VID_DUP_CONNECTION_NAME[]{"AI_MEDIA_DUP_CONNECT"};
  return VID_DUP_CONNECTION_NAME;
#endif
}

} // namespace VidDupHelper
