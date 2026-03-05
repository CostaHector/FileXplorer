#include "FileTool.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "ThumbnailImageViewer.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
#include <QApplication>
#include <QTextStream>
#include <QClipboard>
#include <QDesktopServices>
#include <QProcess>

namespace FileTool {
QByteArray GetLastNLinesOfFile(const QString& logFilePath, const int maxLines) {
  if (maxLines <= 0) {
    return "";
  }

  QFile logFile{logFilePath};
  if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_W("Cannot Open file[%s]: %s", qPrintable(logFile.fileName()), qPrintable(logFile.errorString()));
    return "";
  }

  qint64 fileSize = logFile.size();
  qint64 pos = fileSize - 1;
  int lineCount = 0;
  QByteArray buffer;

  while (pos >= 0 && lineCount <= maxLines) {
    logFile.seek(pos);
    char ch;
    if (!logFile.getChar(&ch)) {
      break;
    }
    if (ch == '\n') {
      lineCount++;
      if (lineCount == maxLines) {
        break;
      }
    }
    pos--;
  }

  // 定位到目标行的起始位置（跳过最后找到的换行符）
  logFile.seek(pos + 1);
  buffer = logFile.readAll();
  logFile.close();
  return buffer;
}

QString TextReader(const QString& textPath, bool* bReadOk) {
  QFile file(textPath);
  if (!file.exists()) {
    LOG_D("File[%s] not found", qPrintable(textPath));
    if (bReadOk != nullptr) {
      *bReadOk = false;
    }
    return "";
  }
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_D("File[%s] open for read failed", qPrintable(textPath));
    if (bReadOk != nullptr) {
      *bReadOk = false;
    }
    return "";
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  QString contents(stream.readAll());
  file.close();
  if (bReadOk != nullptr) {
    *bReadOk = true;
  }
  return contents;
}

QByteArray ByteArrayReader(const QString& baFilePath, bool* bReadOk) {
  QFile baFile(baFilePath);
  if (!baFile.exists()) {
    LOG_D("File[%s] not found", qPrintable(baFilePath));
    if (bReadOk != nullptr) {
      *bReadOk = false;
    }
    return "";
  }
  if (!baFile.open(QIODevice::ReadOnly)) {
    LOG_D("File[%s] open for read failed", qPrintable(baFilePath));
    if (bReadOk != nullptr) {
      *bReadOk = false;
    }
    return "";
  }
  if (bReadOk != nullptr) {
    *bReadOk = true;
  }
  return baFile.readAll();
}

bool TextWriter(const QString& fileName, const QString& content, const QIODevice::OpenMode openMode) {
  QFile fi{fileName};
  if (!fi.open(openMode)) {
    LOG_W("Open [%s] to write failed. mode[%d]", qPrintable(fileName), (int) openMode);
    return false;
  }

  QTextStream stream(&fi); // "\n" will be replace with "\r\n"
  stream.setCodec("UTF-8");
  stream << content;
  stream.flush();
  fi.close();
  return true;
}

bool ByteArrayWriter(const QString& fileName, const QByteArray& ba) {
  QFile fi{fileName};
  if (!fi.open(QIODevice::WriteOnly)) {
    LOG_W("Open [%s] to write failed. fill will not update.", qPrintable(fileName));
    return false;
  }
  QTextStream stream(&fi);
  stream.setCodec("UTF-8"); // must set here
  stream << ba;
  stream.flush();
  fi.close();
  return true;
}

bool OpenLocalFile(const QString& localFilePath) {
  const QFileInfo fi{localFilePath};
  if (!fi.exists()) {
    LOG_WARN_P("Cannot open", "File[%s] not exist.", qPrintable(localFilePath));
    return false;
  }
  if (ThumbnailImageViewer::IsFileAbsPathImage(localFilePath)) {
    return OpenLocalImageFile(localFilePath);
  }
  return OpenLocalFileUsingDesktopService(localFilePath);
}

bool OpenLocalImageFile(const QString& localFilePath) {
  auto* pImageViewer = new (std::nothrow) ThumbnailImageViewer{"IMAGE_VIEWER"};
  QString prepath, name;
  name = PathTool::GetPrepathAndFileName(localFilePath, prepath);
  bool openResult = pImageViewer->setPixmapByAbsFilePath(prepath, name);
  pImageViewer->show();
  return openResult;
}

bool OpenLocalFileUsingDesktopService(const QString& localFilePath) {
  if (!QFile::exists(localFilePath)) {
    LOG_WARN_P("Cannot open", "File[%s] not exist.", qPrintable(localFilePath));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(localFilePath));
}

bool RevealInSystemExplorer(const QString& localFilePath) {
  if (!QFile::exists(localFilePath)) {
    LOG_WARN_NP("Reveal path not exists", localFilePath);
    return false;
  }
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  QProcess process;
  QStringList args;
#ifdef _WIN32
  process.setProgram("explorer.exe");
  args << "/e,"
       << "/select,";
  args << PathTool::sysPath(localFilePath);
#else
  process.setProgram("xdg-open");
  if (QFileInfo(localFilePath).isDir()) {
    args << PathTool::sysPath(localFilePath);
  } else {
    args << PathTool::absolutePath(localFilePath);
  }
#endif
  process.setArguments(args);
  process.startDetached(); // Start the process in detached mode instead of start
  LOG_W("on_revealInExplorer with program[%s] parms [%s]", qPrintable(process.program()), qPrintable(args.join(',')));
  return true;
}

bool CopyTextToSystemClipboard(const QString& text) {
  QClipboard* pClipboard = QApplication::clipboard();
  if (pClipboard == nullptr) {
    LOG_WARN_NP("Cannot copy", "pClipboard copied succeed");
    return false;
  }
  pClipboard->setText(text);
  LOG_OK_P("Copied succeed", "%d char(s)", text.size());
  return true;
}

bool IsTorrentFile(const QString& localFilePath) {
  return localFilePath.endsWith(".torrent", Qt::CaseSensitivity::CaseInsensitive);
}

bool IsTorrentTxtFile(const QString& localFilePath) {
  return localFilePath.endsWith(".torrent.txt", Qt::CaseSensitivity::CaseInsensitive);
}

bool OpenLocalTorrentFile(const QString& localFilePath) {
  // Precondition: program torrent-file-
  // link https://torrent-file-editor.github.io/

  // If it was build it by your self:
  // git clone git@github.com:torrent-file-editor/torrent-file-editor.git
  // set debug path as follows:
  // C:\home\aria\code\torrent-file-editor\build\Debug
  // the program name will be like torrent-file-editor.exe for windows or torrent-file-editor for linux.
  static const QString torrentEditorPath {PathTool::GetPathByApplicationDirPath(PathTool::FILE_REL_PATH::TORRENT_EDITOR_PROG_PATH)};
  // Otherwise download torrent-file-editor-1.0.0-x64.exe and place it in userpath directly
  // static const QString torrentEditorPath{SystemPath::HOME_PATH() + "/torrent-file-editor-1.0.0-x64.exe"};
  if (!QFile::exists(torrentEditorPath)) {
    LOG_D("torrent editor[%s] not exist", qPrintable(torrentEditorPath));
    return false;
  }
  QProcess process;
  QStringList args;
  process.setProgram(torrentEditorPath);
  args << PathTool::sysPath(localFilePath);
  process.setArguments(args);
  process.startDetached(); // Start the process in detached mode instead of start
  return true;
}

} // namespace FileTool

QString ChooseCopyDestination(QString defaultPath, QWidget* parent) {
  if (!QFileInfo(defaultPath).isDir()) {
    defaultPath = Configuration().value(MemoryKey::PATH_LAST_TIME_COPY_TO.name).toString();
  }
  QString selectPath = defaultPath;
#ifndef RUNNING_UNIT_TESTS
  selectPath = QFileDialog::getExistingDirectory(parent, "Choose a destination", defaultPath);
#endif
  QFileInfo dstFi(selectPath); // system may return back slash seperated path
  if (!dstFi.isDir()) {
    LOG_D("selectPath[%s] is not a directory", qPrintable(selectPath));
    return "";
  }
  Configuration().setValue(MemoryKey::PATH_LAST_TIME_COPY_TO.name, dstFi.absoluteFilePath());
  return dstFi.absoluteFilePath();
}

QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG) {
  if (oldAG == nullptr) {
    LOG_C("oldAG is nullptr");
    return "";
  }
  QString i_1_path = first_path;             // first and (i-1) path
  foreach (QAction* act, oldAG->actions()) { // i path
    QString i_path = act->text();
    if (i_path == first_path) {
      act->setText(i_1_path); // finish
      break;
    }
    act->setText(i_1_path);
    i_1_path = i_path;
  }
  QStringList actionList;
  actionList.reserve(oldAG->actions().size());
  foreach (const QAction* act, oldAG->actions()) {
    actionList += act->text();
  }
  return actionList.join('\n');
}

bool LoadCNLanguagePack(QTranslator& translator, QString qmName) {
  if (!translator.load(qmName)) {
    LOG_C("Load language[%s] pack failed", qPrintable(qmName));
    return false;
  }
  LOG_D("Load language[%s] pack succeed", qPrintable(qmName));
  QCoreApplication::installTranslator(&translator);
  return true;
}

bool CreateUserPath() {
  if (QFile::exists(SystemPath::WORK_PATH())) {
    return true;
  }
  if (!QDir{}.mkpath(SystemPath::WORK_PATH())) {
    LOG_C("Create path[%s] failed. Database file of CastView and MovieView cannot located in this path",
          qPrintable(SystemPath::WORK_PATH()));
    return false;
  }
  return true;
}
