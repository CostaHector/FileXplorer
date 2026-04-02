#ifndef QABSTRACTTREEMODELPUB_H
#define QABSTRACTTREEMODELPUB_H

#include "FavoriteItemData.h"
#include <QAbstractItemModel>
#include <memory>

class QAbstractTreeModelPub : public QAbstractItemModel {
 public:
  using QAbstractItemModel::QAbstractItemModel;

  bool setDatas(std::unique_ptr<MyTreeNode> newDatas);
  bool setDatas(const QByteArray& dataByteArray);

  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override { return FavoriteItemData::COLUMN_COUNT; }
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  MyTreeNode* invisibleRootItem() const;

  MyTreeNode* itemFromIndex(const QModelIndex& index) const;

  QModelIndex indexFromItem(const MyTreeNode* item) const;
  static quint16 GetVersion() {
    static constexpr quint16 VERSION = 1;
    return VERSION;
  }
  void setDirty() const { m_bIsDirty = true; }
  void clearDirty() const { m_bIsDirty = false; }
  bool isDirty() const { return m_bIsDirty; }

 protected:
  QByteArray toByteArray() const;

 private:
  std::unique_ptr<MyTreeNode> m_pRoot{nullptr};
  mutable bool m_bIsDirty = false;
};

#endif  // QABSTRACTTREEMODELPUB_H
