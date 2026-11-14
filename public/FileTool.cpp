#include "FileTool.h"
#include "SystemPath.h"
#include "PathTool.h"
#include "ImageTool.h"
#include "PathKey.h"
#include "Configuration.h"
#include "NotificatorMacro.h"
#include "FileImageViewer.h"
#include "DvdFileInfo.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QSaveFile>
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
  if (!logFile.open(QIODevice::ReadOnly)) { // 保持换行符号读取, 不可使用Text, 否则会将文件中的换行符统一替换为\n
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

QString StringTextReader(const QString& textPath, bool* bReadOk) {
  QFile file(textPath);
  if (!file.exists()) {
    LOG_D("File[%s] not found", qPrintable(textPath));
    if (bReadOk != nullptr) {
      *bReadOk = false;
    }
    return "";
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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

bool StringTextWriter(const QString& fileName, const QString& content, const QIODevice::OpenMode openMode) {
  QFile fi{fileName};
  if (!fi.open(openMode)) { // "\n" will be replace with "\r\n" in windows if QIODevice::OpenMode::Text
    LOG_W("Open [%s] to write failed. mode[%d]", qPrintable(fileName), (int) openMode);
    return false;
  }

  QTextStream stream(&fi);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  stream.setEncoding(QStringConverter::Utf8);
#else
  stream.setCodec("UTF-8");
#endif
  stream << content;
  stream.flush();
  fi.close();
  return true;
}

bool CreateABackupFile(const QString& fileName) {
  if (!QFile::exists(fileName)) {
    LOG_D("No need backup. source file[%s] not exist", qPrintable(fileName));
    return true;
  }
  QString backupFileName = fileName;
  backupFileName += QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
  return QFile::copy(fileName, backupFileName);
}

bool AtomicTextWriter(const QString& fileName, const QString& content, const QIODevice::OpenMode openMode) {
  QSaveFile fi{fileName};
  // QSaveFile::open 会在同目录下建临时文件，不会 truncate 原文件
  if (!fi.open(openMode)) {
    LOG_W("Open [%s] for atomic write failed: %s", qPrintable(fileName), qPrintable(fi.errorString()));
    return false;
  }

  // 编码： QSaveFile 不能用 QTextStream + setCodec 了吗？可以，但直接 toUtf8 更简单
  const QByteArray bytes = content.toUtf8();
  const qint64 written = fi.write(bytes);
  if (written != bytes.size()) {
    LOG_W("Partial write [%s]: %lld/%lld (%s)", qPrintable(fileName), static_cast<long long>(written), static_cast<long long>(bytes.size()), qPrintable(fi.errorString()));
    fi.cancelWriting();          // 删掉临时文件，原文件保持不动
    return false;
  }

  // commit 内部流程：flush → fsync(fd) → 原子 rename → 权限/owner 从原文件复制
  if (!fi.commit()) {
    LOG_W("Commit [%s] failed: %s", qPrintable(fileName), qPrintable(fi.errorString()));
    return false;                // 原文件仍然完好
  }
  return true;
}

bool ByteArrayTextWriter(const QString& fileName, const QByteArray& ba) {
  QFile fi{fileName};
  if (!fi.open(QIODevice::WriteOnly)) {
    LOG_W("Open [%s] to write failed. file will not update.", qPrintable(fileName));
    return false;
  }
  QTextStream stream(&fi);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  stream.setEncoding(QStringConverter::Utf8);
#else
  stream.setCodec("UTF-8");
#endif
  stream << ba;
  stream.flush();
  fi.close();
  return true;
}

bool ByteArrayBinaryWriter(const QString& fileName, const QByteArray& ba) {
  QFile dstFi{fileName};
  if (!dstFi.open(QIODevice::WriteOnly)) {
    LOG_W("Open [%s] to write failed. file will not update.", qPrintable(fileName));
    return false;
  }
  dstFi.write(ba);
  dstFi.close();
  return true;
}

bool OpenLocalFile(const QString& localFilePath) {
  const QFileInfo fi{localFilePath};
  if (!fi.exists()) {
    LOG_WARN_P("Cannot open", "File[%s] not exist.", qPrintable(localFilePath));
    return false;
  }
  if (ImageTool::IsFileAbsPathImage(localFilePath)) {
    return OpenLocalImageFile(localFilePath);
  }
  return OpenLocalFileUsingDesktopService(localFilePath);
}

bool OpenLocalImageFile(const QString& localFilePath) {
  if (!QFile::exists(localFilePath)) {
    LOG_WARN_P("Cannot open", "File[%s] not exist.", qPrintable(localFilePath));
    return false;
  }
  auto* pImageViewer = new (std::nothrow) FileImageViewer{"IMAGE_VIEWER"};
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

bool CopyTextToSystemClipboard(const QString& text, bool bAppend) {
  QClipboard* pClipboard = QApplication::clipboard();
  if (pClipboard == nullptr) {
    LOG_WARN_NP("Cannot copy", "pClipboard copied succeed");
    return false;
  }
  if (bAppend) {
    QString newContent{pClipboard->text()};
    if (!newContent.isEmpty()) {
      newContent += '\n';
    }
    newContent += text;
    pClipboard->setText(newContent);
    LOG_OK_P("Copied Append succeed", "%d char(s)", text.size());
  } else {
    pClipboard->setText(text);
    LOG_OK_P("Copied Set succeed", "%d char(s)", text.size());
  }
  return true;
}

bool IsTorrentFile(const QString& localFilePath) {
  return localFilePath.endsWith(".torrent", Qt::CaseSensitivity::CaseInsensitive);
}

bool IsTorrentTxtFile(const QString& localFilePath) {
  return localFilePath.endsWith(".torrent.txt", Qt::CaseSensitivity::CaseInsensitive);
}

qint64 GetFileSize(const QString& fileAbsPath) {
  if (fileAbsPath.endsWith(".dvd", Qt::CaseInsensitive)) {
    return DvdFileInfo::ReadTotalFileSizeFromDvdFile(fileAbsPath);
  }
  return QFile{fileAbsPath}.size();
}

bool OpenLocalTorrentFile(const QString& localFilePath) {
  if (!QFile::exists(localFilePath)) {
    LOG_WARN_P("Cannot open", "File[%s] not exist.", qPrintable(localFilePath));
    return false;
  }
  // Precondition: program torrent-file-
  // link https://torrent-file-editor.github.io/

  // If it was build it by your self:
  // git clone git@github.com:torrent-file-editor/torrent-file-editor.git
  // set debug path as follows:
  // C:\home\aria\code\torrent-file-editor\build\Debug
  // the program name will be like torrent-file-editor.exe for windows or torrent-file-editor for linux.
  static const QString torrentEditorPath{SystemPath::TORRENT_EDITOR_PROG_PATH()};
  // Otherwise download torrent-file-editor-1.0.0-x64.exe and place it in userpath directly
  // static const QString torrentEditorPath{SystemPath::HomePath() + "/torrent-file-editor-1.0.0-x64.exe"};
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
    defaultPath = getConfig(PathKey::LAST_TIME_COPY_TO).toString();
  }
  QString selectPath = QFileDialog::getExistingDirectory(parent, "Choose a destination", defaultPath, QFileDialog::Option::ShowDirsOnly);
  QFileInfo dstFi(selectPath); // system may return back slash seperated path
  if (!dstFi.isDir()) {
    LOG_D("selectPath[%s] is not a directory", qPrintable(selectPath));
    return "";
  }
  setConfig(PathKey::LAST_TIME_COPY_TO, dstFi.absoluteFilePath());
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
