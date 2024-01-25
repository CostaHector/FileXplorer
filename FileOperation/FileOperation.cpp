#include "FileOperation.h"
#include <QDateTime>
const QMap<QString, std::function<FileOperation::RETURN_TYPE(const QStringList&)>> FileOperation::LambdaTable = {
    {"rmfile", rmfileAgent}, {"rmpath", rmpathAgent}, {"rmdir", rmdirAgent},   {"moveToTrash", moveToTrashAgent},
    {"touch", touchAgent},   {"mkpath", mkpathAgent}, {"rename", renameAgent}, {"cpfile", cpfileAgent},
    {"cpdir", cpdirAgent},   {"link", linkAgent},     {"unlink", unlinkAgent}};

auto FileOperation::WriteIntoLogFile(const QString& msg) -> bool {
  QFile logFi(QString("%1/%2.log").arg(PROJECT_PATH+"/bin/runlog").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
  if (not logFi.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    qDebug("log file cannot open. ", logFi.fileName().toStdString().c_str());
    return false;
  }
  QTextStream stream(&logFi);
  stream.setCodec("UTF-8");
  stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\n");
  stream << msg;
  logFi.close();
  return true;
}
