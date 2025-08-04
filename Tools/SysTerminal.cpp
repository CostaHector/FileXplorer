#include "SysTerminal.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QProcess>

#ifdef _WIN32
QString SysTerminal::UpdateBatFile(const QString& command) const {
  using namespace PathTool::FILE_REL_PATH;
  static const QString batFileAbsPath = PathTool::GetPathByApplicationDirPath(TERMINAL_OPEN_BATCH_FILE_PATH);
  QFile fi{batFileAbsPath};
  if (!fi.open(QIODevice::Text | QIODevice::WriteOnly)) {
    qWarning("bat file[%s] open for write failed", qPrintable(batFileAbsPath));
    return "";
  }
  // for chinese path use GB2312. UTF-8 not work in command line
  QTextStream ts(&fi);
  ts.setCodec("GBK2312");
  ts << command;
  fi.close();
  return batFileAbsPath;
}
#endif

bool SysTerminal::operator()(const QString& path) {
  const QFileInfo fi(path);
  if (!fi.exists()) {
    qDebug("path[%s] not exist", qPrintable(path));
    return false;
  }
  const QString pth = QDir::toNativeSeparators(fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath());
#ifdef _WIN32
  static const QString BAT_CONTENT_TEMPLATE{R"(cmd.exe /K "%1 && cd %2")"};
  const int colonIndex = pth.indexOf(':');
  QString batContent;
  if (colonIndex != -1) {
    const QString diskPart = path.left(colonIndex + 1);
    batContent = BAT_CONTENT_TEMPLATE.arg(diskPart, pth);
  } else {
    batContent = BAT_CONTENT_TEMPLATE.arg(pth, pth);
  }
  const QString& batAbsPath = UpdateBatFile(batContent);
  if (batAbsPath.isEmpty()) {
    return false;
  }
  QProcess process;
  process.setProgram("cmd.exe");
  process.setArguments({QDir::toNativeSeparators(batAbsPath)});
  return process.startDetached();
#else
  QProcess process;
  QStringList args;
  process.setProgram("gnome-terminal");
  args << QString("--working-directory=%1").arg(pth);
  process.setArguments(args);
  return process.startDetached();  // Start the process in detached mode instead of start
#endif
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
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

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AwakeSystemTerminal ast;
  ast.show();
  return a.exec();
}
#endif
