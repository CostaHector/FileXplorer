#ifndef MYQSQLTABLEMODEL_H
#define MYQSQLTABLEMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include "PublicVariable.h"

class MyQSqlTableModel : public QSqlTableModel {
 public:
  explicit MyQSqlTableModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());

  auto rootDirectory(const QString& placeHolder = "" /* no use */) -> QDir { return QDir(rootPath(placeHolder)); }

  auto rootPath(const QString& placeHolder = "" /* no use */) -> QString { return placeHolder; }

  auto driver(QModelIndex curIndex) -> QString{
    QModelIndex preIndex = index(curIndex.row(), DB_HEADER_KEY::DB_DRIVER_INDEX, curIndex.parent());
    return data(preIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto absolutePath(QModelIndex curIndex) -> QString {
    QModelIndex preIndex = index(curIndex.row(), DB_HEADER_KEY::DB_PREPATH_INDEX, curIndex.parent());
    return data(preIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto fileName(QModelIndex curIndex) -> QString {
    QModelIndex nameIndex = index(curIndex.row(), DB_HEADER_KEY::DB_NAME_INDEX, curIndex.parent());
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto filePath(QModelIndex curIndex) -> QString { return QDir(absolutePath(curIndex)).absoluteFilePath(fileName(curIndex)); }

  auto fileInfo(QModelIndex curIndex) -> QFileInfo { return QFileInfo(filePath(curIndex)); }
};

#endif  // MYQSQLTABLEMODEL_H
