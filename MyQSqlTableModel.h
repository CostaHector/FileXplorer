#ifndef MYQSQLTABLEMODEL_H
#define MYQSQLTABLEMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include "PublicVariable.h"

class MyQSqlTableModel : public QSqlTableModel {
 public:
  explicit MyQSqlTableModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical) {
      if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
      }
    }
    return QSqlTableModel::headerData(section, orientation, role);
  }

  auto rootDirectory(const QString& placeHolder = "" /* no use */) const -> QDir { return QDir(rootPath(placeHolder)); }

  auto rootPath(const QString& placeHolder = "" /* no use */) const -> QString { return placeHolder; }

  auto driver(QModelIndex curIndex) const -> QString {
    QModelIndex driverIndex = index(curIndex.row(), DB_HEADER_KEY::DB_DRIVER_INDEX, curIndex.parent());
    return data(driverIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto absolutePath(QModelIndex curIndex) const -> QString {
    QModelIndex preIndex = index(curIndex.row(), DB_HEADER_KEY::DB_PREPATH_INDEX, curIndex.parent());
    return data(preIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto fileName(QModelIndex curIndex) const -> QString {
    QModelIndex nameIndex = index(curIndex.row(), DB_HEADER_KEY::DB_NAME_INDEX, curIndex.parent());
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto filePath(QModelIndex curIndex) const -> QString { return QDir(absolutePath(curIndex)).absoluteFilePath(fileName(curIndex)); }

  auto fileInfo(QModelIndex curIndex) const -> QFileInfo { return QFileInfo(filePath(curIndex)); }
};

#endif  // MYQSQLTABLEMODEL_H
