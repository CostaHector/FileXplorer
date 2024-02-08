#include "SysTerminal.h"
#include <PublicVariable.h>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>

SysTerminal::SysTerminal() {
#ifdef _WIN32
  m_BAT_FILE_PATH = PreferenceSettings().value(MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.name).toString();
#else
  m_BAT_FILE_PATH = PreferenceSettings().value(MemoryKey::LINUX_TERMINAL_OPEN_BATCH_FILE_PATH.name).toString();
#endif
}

bool SysTerminal::WriteIntoBatchFile(const QString& command) {
  QFile fi(m_BAT_FILE_PATH);
  if (not fi.open(QIODevice::Text | QIODevice::WriteOnly)) {
    qWarning("Cannot write into [%s]. May write protected or folder not exists", qPrintable(m_BAT_FILE_PATH));
    return false;
  }
  // for chinese path use GB2312. UTF-8 not work in command line
  QTextStream ts(&fi);
  ts.setCodec("GBK2312");
  ts << command;
  fi.close();
  return QDesktopServices::openUrl(QUrl::fromLocalFile(m_BAT_FILE_PATH));
}

bool SysTerminal::operator()(const QString& path) {
#ifdef _WIN32
  QFileInfo fi(path);
  if (not fi.exists()) {
    qDebug("path[%s] not exist", qPrintable(path));
    return false;
  }
  QString containerPath = fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath();
  const int colonIndex = containerPath.indexOf(':');
  QString command;
  if (colonIndex != -1) {
    QString diskPart = path.left(colonIndex + 1);
    command = QString("cmd.exe /K \"%1 && cd %2\"").arg(diskPart, path);
  } else {
    command = QString("cmd.exe /K \"%1\"").arg(path);
  }
  return WriteIntoBatchFile(command);
#else
  qDebug("Only Support in WINDOWS NOW");
  return false;
#endif
}

#include <QLineEdit>
class AwakeSystemTerminal : public QLineEdit {
 public:
  explicit AwakeSystemTerminal(QWidget* parent = nullptr) : QLineEdit(QFileInfo(__FILE__).absoluteFilePath(), parent) {
    setWindowTitle("Awake System Terminal");
    connect(this, &QLineEdit::returnPressed, this, &AwakeSystemTerminal::Awake);
  }

  auto Awake() -> bool { return SysTerminal()(text()); }
  QSize sizeHint() const override { return QSize(1024, 768); }
};

//#define __NAME__EQ__MAIN__ 1

#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AwakeSystemTerminal ast;
  ast.show();
  return a.exec();
}
#endif
