#include "CopyItemPropertiesToClipboardIF.h"
#include "ViewSelection.h"

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

bool CopyItemPropertiesToClipboardIF::on_copySelectedItemFullPath(const QAbstractItemView* view, QFileSystemModel* fileSysModel) {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->filePath(ind));
  }
  return PathCopyTriple(mixed, "absolute-file-path");
}

bool CopyItemPropertiesToClipboardIF::on_copySelectedItemFullPath(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->filePath(ind));
  }
  return PathCopyTriple(mixed, "absolute-file-path");
}

auto CopyItemPropertiesToClipboardIF::on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view, QFileSystemModel* fileSysModel)
    -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    const QFileInfo dirFi = fileSysModel->fileInfo(ind);
    const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
    mixed.append(QDir::toNativeSeparators(imagePath));
  }
  return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
}

auto CopyItemPropertiesToClipboardIF::on_copyFullPathFolderNameAndAppendImageSuffix(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel)
    -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    const QFileInfo dirFi = fileSysModel->fileInfo(ind);
    const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
    mixed.append(QDir::toNativeSeparators(imagePath));
  }
  return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
}

auto CopyItemPropertiesToClipboardIF::on_copyDirPath(const QAbstractItemView* view, QFileSystemModel* fileSysModel) -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->fileInfo(ind).absolutePath());
  }
  return PathCopyTriple(mixed, "absolute-path");
}

auto CopyItemPropertiesToClipboardIF::on_copyDirPath(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->absolutePath(ind));
  }
  return PathCopyTriple(mixed, "absolute-path");
}

auto CopyItemPropertiesToClipboardIF::on_copyName(const QAbstractItemView* view, QFileSystemModel* fileSysModel) -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->fileName(ind));
  }
  return PathCopyTriple(mixed, "file-name");
}

auto CopyItemPropertiesToClipboardIF::on_copyName(const QAbstractItemView* view, MyQSqlTableModel* fileSysModel) -> bool {
  QStringList mixed;
  for (const QModelIndex ind : selectedIndexes(view)) {
    mixed.append(fileSysModel->fileName(ind));
  }
  return PathCopyTriple(mixed, "file-name");
}
