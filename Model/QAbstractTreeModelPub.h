#ifndef QABSTRACTTREEMODELPUB_H
#define QABSTRACTTREEMODELPUB_H

#include <QAbstractItemModel>
#include <memory>

template<typename TDataType>
class QAbstractTreeModelPub : public QAbstractItemModel {
 public:
  using QAbstractItemModel::QAbstractItemModel;

  bool setDatas(std::unique_ptr<TDataType> newDatas);
  bool setDatas(const QByteArray& dataByteArray);

  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex siblingAtColumn(const QModelIndex &childIndex, int siblingColumn) const;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override { return TDataType::columnCount(); }
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  TDataType* invisibleRootItem() const;

  TDataType* itemFromIndex(const QModelIndex& index) const;

  QModelIndex indexFromItem(const TDataType* item) const;
  static quint16 GetVersion() {
    static constexpr quint16 VERSION = 1;
    return VERSION;
  }
  void setDirty() const { m_bIsDirty = true; }
  void clearDirty() const { m_bIsDirty = false; }
  bool isDirty() const { return m_bIsDirty; }

  bool isRoot(const QModelIndex& parentIndex) const { return !parentIndex.isValid(); }
  bool isGroup(const QModelIndex& parentIndex) const;
  QString groupName(const QModelIndex& parentIndex) const;
  bool onRename(const QModelIndex& parentIndex, const QString& newName);

  static bool isIndexValidAndDescendantOfValidAncestor(const QModelIndex& descendant, const QModelIndex& ancestor);
  int moveParentIndexesTo(const QModelIndexList& parentIndexes, const QModelIndex& dest);
  int removeParentIndexes(const QModelIndexList& parentIndexes);

  TDataType* canDropIntoIndex(const QModelIndex& destParentIndex) const;

  TDataType* addGroup(const QString& grpName, const QModelIndex& parentIndex = {});
  TDataType* addGroup(const QString& grpName, TDataType* parentItem = nullptr);

 protected:
  QByteArray toByteArray() const;

 private:
  static bool hasAncestorInSet(const TDataType* node, const QSet<TDataType*>& ancestorSet);
  static QList<TDataType*> filterWithoutAncestor(const QList<TDataType*>& items, const QSet<TDataType*>& ancestorSet);

  QList<TDataType*> GetItemsNeedProcess(const QModelIndexList& parentIndexes, TDataType* destItem) const;
  std::unique_ptr<TDataType> m_pRoot{nullptr};
  mutable bool m_bIsDirty = false;
};

#endif  // QABSTRACTTREEMODELPUB_H
