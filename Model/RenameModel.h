#ifndef RENAMEMODEL_H
#define RENAMEMODEL_H

#include "QAbstractTableModelPub.h"
#include "public/PathTool.h"

class RenameModel : public QAbstractTableModelPub {
 public:
  explicit RenameModel(QObject* object = nullptr);
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mOsWalker.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return RENAME_HOR_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override {  //
    if (index.column() < WRITE_PROTECT_COLUMN) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }
  void SetNewCompleteNames(QStringList newCompleteNames);
  void SetNewOsWalker(FileOsWalker osWalker);

 private:
  FileOsWalker mOsWalker;
  QStringList newNames;
  QStringList newExts;
  static const QStringList RENAME_HOR_HEADER;
  static constexpr int WRITE_PROTECT_COLUMN = 3;
};

#endif  // RENAMEMODEL_H
