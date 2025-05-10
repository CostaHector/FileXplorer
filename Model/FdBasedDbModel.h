#ifndef FDBASEDDBMODEL_H
#define FDBASEDDBMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include <QDir>
#include "Tools/FileDescriptor/TableFields.h"

class FdBasedDbModel : public QSqlTableModel {
 public:
  explicit FdBasedDbModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());

  void setTable(const QString& tableName) override;

  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical) {
      if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
      }
    }
    return QSqlTableModel::headerData(section, orientation, role);
  }

  QDir rootDirectory(const QString& /*placeHolder*/ = "") const { return QDir(rootPath()); }

  QString rootPath(const QString& /*placeHolder*/ = "") const { return m_rootPath; }

  QString GUID(const QModelIndex& /*curIndex*/ = {}) const { return tableName().replace('_', '-'); }

  QString absolutePath(const QModelIndex& curIndex) const;

  QString fileName(const QModelIndex& curIndex) const {
    const QModelIndex& nameIndex = curIndex.siblingAtColumn(MOVIE_TABLE::Name);
    return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
  }

  QString filePath(const QModelIndex& curIndex) const {  //
    return QDir{absolutePath(curIndex)}.absoluteFilePath(fileName(curIndex));
  }

  QFileInfo fileInfo(const QModelIndex& curIndex) const {  //
    return QFileInfo{filePath(curIndex)};
  }

  QString fullInfo(const QModelIndex& curIndex) const {
    return data(curIndex.siblingAtColumn(MOVIE_TABLE::Name)).toString()    //
           + '\t'                                                          //
           + data(curIndex.siblingAtColumn(MOVIE_TABLE::Size)).toString()  //
           + '\t'                                                          //
           + data(curIndex.siblingAtColumn(MOVIE_TABLE::PrePathRight)).toString();
  }

  void SetStudio(const QModelIndexList& tagColIndexes, const QString& studio);
  void SetCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence);
  void AddCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence);
  void RmvCastOrTags(const QModelIndexList& tagColIndexes, const QString& cast);

 private:
  QString m_rootPath;
};

#endif  // FDBASEDDBMODEL_H
