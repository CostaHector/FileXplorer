#include "CopyItemPropertiesToClipboardIF.h"
#include "Model/FdBasedDbModel.h"
#include "ViewSelection.h"

#include <QFileSystemModel>

#include <QApplication>
#include <QClipboard>

using ViewSelection::selectedIndexes;

auto CopyItemPropertiesToClipboardIF::PathCopyTriple(const QStringList& lst, const QString& opName) -> bool {
  if (lst.isEmpty()) {
    qDebug("NOTHING %s copied. clipboard state kept.", qPrintable(opName));
    return true;
  }
  const QString& msg = QString("[%1] letter(s) has been [%2].").arg(lst.size()).arg(opName);
  const QString& copiedStr = lst.join('\n');
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  qDebug("%s", qPrintable(msg));
  return true;
}

bool CopyItemPropertiesToClipboardIF::on_copySelectedItemFullPath(const QAbstractItemView* view) {
  QStringList mixed;
  auto* sqlModel = dynamic_cast<const FdBasedDbModel*>(view->model());
  if (sqlModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(sqlModel->filePath(ind));
    }
    return PathCopyTriple(mixed, "absolute-file-path");
  }
  auto* fsmModel = dynamic_cast<const QFileSystemModel*>(view->model());
  if (fsmModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fsmModel->filePath(ind));
    }
  }
  return PathCopyTriple(mixed, "absolute-file-path");
}

auto CopyItemPropertiesToClipboardIF::on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view)
    -> bool {
  QStringList mixed;
  auto* sqlModel = dynamic_cast<const FdBasedDbModel*>(view->model());
  if (sqlModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      const QFileInfo dirFi = sqlModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      mixed.append(QDir::toNativeSeparators(imagePath));
    }
    return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
  }
  auto* fsmModel = dynamic_cast<const QFileSystemModel*>(view->model());
  if (fsmModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      const QFileInfo dirFi = fsmModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      mixed.append(QDir::toNativeSeparators(imagePath));
    }
  }
  return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
}

auto CopyItemPropertiesToClipboardIF::on_copyDirPath(const QAbstractItemView* view) -> bool {
  QStringList mixed;
  auto* sqlModel = dynamic_cast<const FdBasedDbModel*>(view->model());
  if (sqlModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(sqlModel->fileInfo(ind).absolutePath());
    }
    return PathCopyTriple(mixed, "absolute-path");
  }
  auto* fsmModel = dynamic_cast<const QFileSystemModel*>(view->model());
  if (fsmModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fsmModel->fileInfo(ind).absolutePath());
    }
  }
  return PathCopyTriple(mixed, "absolute-path");
}

auto CopyItemPropertiesToClipboardIF::on_copyName(const QAbstractItemView* view) -> bool {
  QStringList mixed;
  auto* sqlModel = dynamic_cast<const FdBasedDbModel*>(view->model());
  if (sqlModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(sqlModel->fileName(ind));
    }
    return PathCopyTriple(mixed, "file-name");
  }
  auto* fsmModel = dynamic_cast<const QFileSystemModel*>(view->model());
  if (fsmModel) {
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fsmModel->fileName(ind));
    }
  }
  return PathCopyTriple(mixed, "file-name");
}


