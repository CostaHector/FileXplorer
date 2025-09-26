#include "PublicTool.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "Logger.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QTextStream>
#include <QPixmap>
#include <QPainter>

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
    if (bReadOk != nullptr) *bReadOk = false;
    return "";
  }
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_D("File[%s] open for read failed", qPrintable(textPath));
    if (bReadOk != nullptr) *bReadOk = false;
    return "";
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  QString contents(stream.readAll());
  file.close();
  if (bReadOk != nullptr) *bReadOk = true;
  return contents;
}

bool TextWriter(const QString& fileName, const QString& content, const QIODevice::OpenMode openMode) {
  QFile fi{fileName};
  if (!fi.open(openMode)) {
    LOG_W("Open [%s] to write failed. mode[%d]", qPrintable(fileName), (int)openMode);
    return false;
  }
  QTextStream stream(&fi);
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
  stream.setCodec("UTF-8");  // must set here
  stream << ba;
  stream.flush();
  fi.close();
  return true;
}

bool OpenLocalFileUsingDesktopService(const QString& localFilePath) {
  if (!QFile::exists(localFilePath)) {
    LOG_W("Cannot open. File [%s] not exist.", qPrintable(localFilePath));
    return false;
  }
#ifndef RUNNING_UNIT_TESTS
  return QDesktopServices::openUrl(QUrl::fromLocalFile(localFilePath));
#else
  return true;
#endif
}

QPixmap GetRatePixmap(const int r, const int sliceCount, const bool hasBorder) {
  if (r < 0 || r > sliceCount) {
    LOG_D("rate[%d] out bound", r);
    return {};
  }
  static constexpr int WIDTH = 100, HEIGHT = (int)(WIDTH * 0.618);
  QPixmap mp{WIDTH, HEIGHT};
  int orangeWidth = WIDTH * r / sliceCount;
  static constexpr QColor OPAGUE{0, 0, 0, 0};
  static constexpr QColor STD_ORANGE{255, 165, 0, 255};
  mp.fill(OPAGUE); // opague
  QPainter painter{&mp};
  painter.setPen(STD_ORANGE); // standard orange
  painter.setBrush(STD_ORANGE);
  painter.drawRect(0, 0, orangeWidth, HEIGHT);
  if (hasBorder) {
    painter.setPen(QColor{0, 0, 0, 255}); // standard black
    painter.setBrush(OPAGUE);
    painter.drawRect(0, 0, WIDTH - 1, HEIGHT - 1);
  }
  painter.end();
  return mp;
}

}  // namespace FileTool

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
  QString i_1_path = first_path;              // first and (i-1) path
  foreach (QAction* act, oldAG->actions()) {  // i path
    QString i_path = act->text();
    if (i_path == first_path) {
      act->setText(i_1_path);  // finish
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
#ifndef RUNNING_UNIT_TESTS
  qmName = PROJECT_NAME "_zh_CN.qm";
#endif
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
    LOG_C("Create path[%s] failed. Database file of CastView and MovieView cannot located in this path", qPrintable(SystemPath::WORK_PATH()));
    return false;
  }
  return true;
}
