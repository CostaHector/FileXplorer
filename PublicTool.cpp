#include "PublicTool.h"
#include <QDir>

PublicTool::PublicTool() {}

bool PublicTool::copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist) {
  QDir sourceDir(fromDir);
  QDir targetDir(toDir);
  if (!targetDir.exists()) { /* if directory don't exists, build it */
    if (!targetDir.mkdir(targetDir.absolutePath()))
      return false;
  }

  QFileInfoList fileInfoList = sourceDir.entryInfoList();
  for (const QFileInfo& fileInfo : fileInfoList) {
    if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
      continue;

    if (fileInfo.isDir()) { /* if it is directory, copy recursively*/
      if (copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)) {
        continue;
      }
      return false;
    }
    /* if coverFileIfExist == true, remove old file first */

    if (targetDir.exists(fileInfo.fileName())) {
      if (coverFileIfExist) {
        targetDir.remove(fileInfo.fileName());
        qDebug("%s/%s is covered by file under [%s]", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str(),
               fromDir.toStdString().c_str());
      } else {
        qDebug("%s/[%s] was kept", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str());
      }
    }
    // files copy
    if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
      return false;
    }
  }
  return true;
}

#include <QDir>
#include <QDirIterator>

OSWalker_RETURN OSWalker(const QString& pre, const QStringList& rels, const bool includingSub, const bool includingSuffix) {
  // Reverse the return value, One can get bottom To Top result like os.walk
  QDirIterator::IteratorFlag dirIterFlag = includingSub ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
  const int n1 = pre.size() + 1;
  QDir preDir(pre);

  QStringList relToNames;
  QStringList completeNames;
  QStringList suffixs;
  QList<bool> isFiles;

  for (const QString& rel : rels) {
    QFileInfo fileInfo(preDir.absoluteFilePath(rel));
    isFiles.append(fileInfo.isFile());
    relToNames.append(fileInfo.absolutePath().mid(n1));
    if (includingSuffix) {
      completeNames.append(fileInfo.fileName());
      suffixs.append("");
    } else {
      completeNames.append(fileInfo.completeBaseName());
      suffixs.append(fileInfo.suffix());
    }

    if (includingSub and fileInfo.isDir()) {  // folders
      QDirIterator it(fileInfo.absoluteFilePath(), {}, QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, dirIterFlag);
      while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        isFiles.append(fi.isFile());
        relToNames.append(fi.absolutePath().mid(n1));
        if (includingSuffix) {
          completeNames.append(fi.fileName());
          suffixs.append("");
        } else {
          completeNames.append(fi.completeBaseName());
          suffixs.append(fi.suffix());
        }
      }
    }
  }
  return {relToNames, completeNames, suffixs, isFiles};
}

auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction* {
  if (ag == nullptr) {
    return nullptr;
  }
  for (QAction* act : ag->actions()) {
    if (act->text() == actionName) {
      return act;
    }
  }
  return nullptr;
}

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align) {
  for (int i = 0; i < lay->count(); ++i) {
    lay->itemAt(i)->setAlignment(align);
  }
  // Only QToolBar and QToolButton need to set alignment. (QWidget like QSeperator not need)
}

auto Walker(const QString& preUserInput, const QStringList& rels) -> QStringList {
  int n1 = -1;
  QString pre;
  if (not preUserInput.isEmpty()) {
    const QString& preNative = QDir::fromNativeSeparators(preUserInput);
    pre = QDir(preNative).absolutePath();
    n1 = pre.size() + 1;
  } else {  // please do it manually in explorer;
    n1 = 0;
  }
  QDir preDir(pre);
  QStringList relFullNames;

  for (const QString& rel : rels) {
    QFileInfo fileInfo(preDir.absoluteFilePath(rel));
    const QString& relPath = fileInfo.absoluteFilePath().mid(n1);
    if (fileInfo.isDir()) {  // folders;
      QDirIterator it(fileInfo.absoluteFilePath(), {}, QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries,
                      QDirIterator::IteratorFlag::Subdirectories);
      while (it.hasNext()) {
        it.next();
        relFullNames.append(it.filePath().mid(n1));
      }
    }
    relFullNames.append(relPath);
  }
  return {relFullNames.crbegin(), relFullNames.crend()};
}
