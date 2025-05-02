#ifndef MYQSQLTABLEMODEL_H
#define MYQSQLTABLEMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include <QDir>
#include "Tools/FileDescriptor/TableFields.h"

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
    const QModelIndex& driverIndex = curIndex.siblingAtColumn(DB_HEADER_KEY::Driver);
    return data(driverIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto absolutePath(const QModelIndex& curIndex) const -> QString {
    const QModelIndex& preIndex = curIndex.siblingAtColumn(DB_HEADER_KEY::Prepath);
    return data(preIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto fileName(const QModelIndex& curIndex) const -> QString {
    const QModelIndex& nameIndex = curIndex.siblingAtColumn(DB_HEADER_KEY::Name);
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  auto filePath(const QModelIndex& curIndex) const -> QString {  //
    return QDir{absolutePath(curIndex)}.absoluteFilePath(fileName(curIndex));
  }

  auto fileInfo(const QModelIndex& curIndex) const -> QFileInfo {  //
    return QFileInfo{filePath(curIndex)};
  }

  auto fullInfo(const QModelIndex& curIndex) const -> QString {
    using namespace DB_HEADER_KEY;
    return data(curIndex.siblingAtColumn(DB_HEADER_KEY::Name)).toString()    //
           + '\t'                                                         //
           + data(curIndex.siblingAtColumn(DB_HEADER_KEY::Size)).toString()  //
           + '\t'                                                         //
           + data(curIndex.siblingAtColumn(DB_HEADER_KEY::Prepath)).toString();
  }
};

#endif  // MYQSQLTABLEMODEL_H
