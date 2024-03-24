#ifndef DIFFERROOTFILESYSTEMMODEL_H
#define DIFFERROOTFILESYSTEMMODEL_H

// DifferRootFileSystemModel, a model contains files/folders from different path.
#include <QAbstractListModel>
class DifferRootFileSystemModel : public QAbstractListModel {
 public:
  DifferRootFileSystemModel(QObject* parent = nullptr);

  virtual int appendAPath(const QString& path) = 0;
  virtual int appendRows(const QStringList& lst) = 0;

  virtual QString filePath(const QModelIndex& index) const = 0;
  virtual QString filePath(const int row) const = 0;

  virtual void clear() = 0;

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == 0) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }
};

#endif  // DIFFERROOTFILESYSTEMMODEL_H
