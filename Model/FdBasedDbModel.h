#ifndef FDBASEDDBMODEL_H
#define FDBASEDDBMODEL_H

#include "SqlTableModelPub.h"
#include <QFileInfo>
#include <QDir>

class FdBasedDbModel : public SqlTableModelPub {
public:
  explicit FdBasedDbModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());

  void setTable(const QString& tableName) override;
  static void setDataStatic(FdBasedDbModel& self, const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) {
    self.setData(index, value, role);
  }
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical && role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    }
    return QSqlTableModel::headerData(section, orientation, role);
  }

  QDir rootDirectory(const QString& /*placeHolder*/ = "") const { return QDir{rootPath()}; }

  QString rootPath(const QString& /*placeHolder*/ = "") const { return m_rootPath; }

  QString GUID(const QModelIndex& /*curIndex*/ = {}) const { return tableName().replace('_', '-'); }

  QString absolutePath(const QModelIndex& curIndex) const;

  QString fileName(const QModelIndex& curIndex) const;

  QString filePath(const QModelIndex& curIndex) const {  //
    if (!curIndex.isValid()) {
      return {};
    }
    return QDir{absolutePath(curIndex)}.absoluteFilePath(fileName(curIndex));
  }

  QFileInfo fileInfo(const QModelIndex& curIndex) const {  //
    if (!curIndex.isValid()) {
      return {};
    }
    return QFileInfo{filePath(curIndex)};
  }

  QString fullInfo(const QModelIndex& curIndex) const;

  void SetStudio(const QModelIndexList& tagColIndexes, const QString& studio);
  void SetCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence);
  void AddCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence);
  void RmvCastOrTags(const QModelIndexList& tagColIndexes, const QString& cast);

private:
  QString m_rootPath;
};

#endif  // FDBASEDDBMODEL_H
