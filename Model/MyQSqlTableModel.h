#ifndef MYQSQLTABLEMODEL_H
#define MYQSQLTABLEMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include "public/PublicVariable.h"

class MyQSqlTableModel : public QSqlTableModel {
 public:
  explicit MyQSqlTableModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());


  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;

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

  auto driver(const QModelIndex& curIndex) const -> QString {
    const QModelIndex& driverIndex = index(curIndex.row(), DB_HEADER_KEY::DB_DRIVER_INDEX, curIndex.parent());
    return data(driverIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto absolutePath(const QModelIndex& curIndex) const -> QString {
    const QModelIndex& preIndex = index(curIndex.row(), DB_HEADER_KEY::DB_PREPATH_INDEX, curIndex.parent());
    return data(preIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto fileName(const QModelIndex& curIndex) const -> QString {
    const QModelIndex& nameIndex = index(curIndex.row(), DB_HEADER_KEY::DB_NAME_INDEX, curIndex.parent());
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto filePath(const QModelIndex& curIndex) const -> QString { return QDir(absolutePath(curIndex)).absoluteFilePath(fileName(curIndex)); }

  auto fileInfo(const QModelIndex& curIndex) const -> QFileInfo { return QFileInfo(filePath(curIndex)); }

  auto fullInfo(const QModelIndex& curIndex) const -> QString {
    using namespace DB_HEADER_KEY;
    const int row = curIndex.row();
    const QModelIndex& par = curIndex.parent();
    return data(index(row, DB_NAME_INDEX, par)).toString() + '\t' + data(index(row, DB_SIZE_COLUMN, par)).toString() + '\t' +
           data(index(row, DB_PREPATH_INDEX, par)).toString();
  }
};

#endif  // MYQSQLTABLEMODEL_H
