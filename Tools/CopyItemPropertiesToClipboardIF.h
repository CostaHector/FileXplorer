#ifndef COPYITEMPROPERTIESTOCLIPBOARDIF_H
#define COPYITEMPROPERTIESTOCLIPBOARDIF_H

#include <QApplication>
#include <QClipboard>
#include <QFileSystemModel>
#include <QListView>
#include <QTableView>

#include "MyQSqlTableModel.h"

class CopyItemPropertiesToClipboardIF {
 public:
  CopyItemPropertiesToClipboardIF();

  static auto selectedIndexes(const QListView* view) -> QModelIndexList {
    if (view == nullptr) {
      return {};
    }
    return view->selectionModel()->selectedIndexes();
  }

  static auto selectedIndexes(const QTableView* view) -> QModelIndexList {
    if (view == nullptr) {
      return {};
    }
    return view->selectionModel()->selectedRows();
  }

  static auto PathCopyTriple(const QStringList& lst, const QString& opName) -> bool {
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

  static bool on_copySelectedItemFullPath(const QTableView* view, QFileSystemModel* fileSysModel) {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->filePath(ind));
    }
    return PathCopyTriple(mixed, "absolute-file-path");
  }

  static bool on_copySelectedItemFullPath(const QTableView* view, MyQSqlTableModel* fileSysModel) {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->filePath(ind));
    }
    return PathCopyTriple(mixed, "absolute-file-path");
  }

  static auto on_copyFullPathFolderNameAndAppendImageSuffix(const QTableView* view, QFileSystemModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      const QFileInfo dirFi = fileSysModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      mixed.append(QDir::toNativeSeparators(imagePath));
    }
    return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
  }
  static auto on_copyFullPathFolderNameAndAppendImageSuffix(const QTableView* view, MyQSqlTableModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      const QFileInfo dirFi = fileSysModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      mixed.append(QDir::toNativeSeparators(imagePath));
    }
    return PathCopyTriple(mixed, "absolute-file-path+folderName+.jpg(in local seperator)");
  }


  static auto on_copyDirPath(const QTableView* view, QFileSystemModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->fileInfo(ind).absolutePath());
    }
    return PathCopyTriple(mixed, "absolute-path");
  }
  static auto on_copyDirPath(const QTableView* view, MyQSqlTableModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->absolutePath(ind));
    }
    return PathCopyTriple(mixed, "absolute-path");
  }

  static auto on_copyName(const QTableView* view, QFileSystemModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->fileName(ind));
    }
    return PathCopyTriple(mixed, "file-name");
  }
  static auto on_copyName(const QTableView* view, MyQSqlTableModel* fileSysModel) -> bool {
    QStringList mixed;
    for (const QModelIndex ind : selectedIndexes(view)) {
      mixed.append(fileSysModel->fileName(ind));
    }
    return PathCopyTriple(mixed, "file-name");
  }
};

#endif  // COPYITEMPROPERTIESTOCLIPBOARDIF_H
