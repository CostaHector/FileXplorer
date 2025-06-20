﻿#include "public/PublicTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

#include <QAction>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG) {
  if (oldAG == nullptr) {
    qCritical("oldAG is nullptr");
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

QString TextReader(const QString& textPath) {
  QFile file(textPath);
  if (!file.exists()) {
    qDebug("File[%s] not found", qPrintable(textPath));
    return "";
  }
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug("File[%s] open for read failed", qPrintable(textPath));
    return "";
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  QString contents(stream.readAll());
  file.close();
  return contents;
}

bool TextWriter(const QString& fileName, const QString& content, const QIODevice::OpenMode openMode) {
  QFile fi{fileName};
  if (!fi.open(openMode)) {
    qWarning("Open [%s] to write failed. fill will not update.", qPrintable(fileName));
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
    qWarning("Open [%s] to write failed. fill will not update.", qPrintable(fileName));
    return false;
  }
  QTextStream stream(&fi);
  stream.setCodec("UTF-8"); // must set here
  stream << ba;
  stream.flush();
  fi.close();
  return true;
}

QString ChooseCopyDestination(QString defaultPath, QWidget* parent) {
  if (!QFileInfo(defaultPath).isDir()) {
    defaultPath = PreferenceSettings().value(MemoryKey::PATH_LAST_TIME_COPY_TO.name).toString();
  }
  const auto selectPath = QFileDialog::getExistingDirectory(parent, "Choose a destination", defaultPath);
  QFileInfo dstFi(selectPath);
  if (!dstFi.isDir()) {
    qDebug("selectPath[%s] is not a directory", qPrintable(selectPath));
    return "";
  }
  PreferenceSettings().setValue(MemoryKey::PATH_LAST_TIME_COPY_TO.name, dstFi.absoluteFilePath());
  return dstFi.absoluteFilePath();
}

void LoadCNLanguagePack(QTranslator& translator) {
  qDebug("Load Chinese pack");
  const QString baseName = "FileXplorer_zh_CN";
  if (translator.load(":/i18n/" + baseName)) {
    qDebug("Load language pack succeed %s", qPrintable(baseName));
    QCoreApplication::installTranslator(&translator);
  }
}

void LoadSysLanaguagePack(QTranslator& translator) {
  qDebug("Load System Language pack");
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString& locale : uiLanguages) {
    const QString baseName = "FileXplorer" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      qDebug("Load language pack succeed %s", qPrintable(baseName));
      QCoreApplication::installTranslator(&translator);
      break;
    } else {
      qDebug("No need to load language pack %s", qPrintable(baseName));
    }
  }
}
