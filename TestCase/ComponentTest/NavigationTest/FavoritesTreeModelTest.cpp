#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "PublicVariable.h"

#include "BeginToExposePrivateMember.h"
#include "FavoritesTreeModel.h"
#include "EndToExposePrivateMember.h"

#include <QMimeData>

class FavoritesTreeModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {  //
    Configuration().remove("BelongToFavoritesTreeView_DATAS");
  }

  void default_construct_ok() {
    {  // initialCollections: 2 add group, 1 add path
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, true};
      QCOMPARE(model.rowCount({}), 3);  // count(root)=3
      model.saveToSettings();
      QCOMPARE(Configuration().contains("BelongToFavoritesTreeView_DATAS"), true);

      QCOMPARE(model.rowCount(model.index(0, 0, {})), 2);
      QCOMPARE(model.rowCount(model.index(1, 0, {})), 2);
      QCOMPARE(model.rowCount(model.index(2, 0, {})), 0);

      QCOMPARE(model.isRoot(QModelIndex{}), true);
      QCOMPARE(model.isGroup(QModelIndex{}), false);  // root not regard as group
      QCOMPARE(model.isGroup(model.index(0, 0, {})), true);
      QCOMPARE(model.isGroup(model.index(1, 0, {})), true);
      QCOMPARE(model.isGroup(model.index(2, 0, {})), false);

      QCOMPARE(model.filePath({}), "");
      QCOMPARE(model.filePath(model.index(0, 0, {})), "");
      QCOMPARE(model.filePath(model.index(1, 0, {})), "");
      QCOMPARE(model.filePath(model.index(2, 0, {})), SystemPath::HOME_PATH() + "/code");

      QVERIFY(model.addPath("random name", "path/to/random name", nullptr) != nullptr);
      QCOMPARE(model.rowCount({}), 4);  // count(root)=3+1
      QCOMPARE(model.itemFromIndex(model.index(3, 0, {}))->text(), "random name");
      QCOMPARE(model.isGroup(model.index(3, 0, {})), false);
      QCOMPARE(model.filePath(model.index(3, 0, {})), "path/to/random name");
      // will call save to in destruction automatically
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView_DATAS"), true);

    {  // no initialCollections, vector<FavoriteItemData>.size() == 4
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.rowCount({}), 4);  // count(root)=3+1
      QCOMPARE(model.itemFromIndex(model.index(3, 0, {}))->text(), "random name");
      QCOMPARE(model.isGroup(model.index(3, 0, {})), false);
      QCOMPARE(model.filePath(model.index(3, 0, {})), "path/to/random name");

      QVERIFY(model.setDatas(QByteArray{}));
      QCOMPARE(model.rowCount({}), 0);  // cleared
      // will call save to in destruction automatically
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView_DATAS"), true);

    {  // no initialCollections, vector<FavoriteItemData>.size() == 0
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.rowCount({}), 0);  // count(root)=0
      model.setThisTimeNotSave(true);
      Configuration().remove("BelongToFavoritesTreeView_DATAS");
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView_DATAS"), false);

    {  // no initialCollections, no key exist
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.mNotSaveDatasThisTimeBeforeDestruct, false);
      QCOMPARE(model.rowCount({}), 0);  // count(root)=0
      model.setThisTimeNotSave(true);
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView_DATAS"), false);
  }

  void addGroup_ok() {
    Configuration().remove("BelongToFavoritesTreeView_DATAS");
    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.mNotSaveDatasThisTimeBeforeDestruct, false);
    QCOMPARE(model.rowCount({}), 0);  // count(root)=0

    auto* pItem0 = model.addGroup("grpInRoot0", QModelIndex{});
    QVERIFY(pItem0 != nullptr);
    QCOMPARE(model.rowCount({}), 1);
    const QModelIndex item0Ind{model.index(0, 0, {})};
    QCOMPARE(pItem0->rowCount(), 0);
    auto* pItem0_0 = model.addGroup("grpInRoot0_child", item0Ind);
    QVERIFY(pItem0_0 != nullptr);
    QCOMPARE(pItem0->rowCount(), 1);
    QCOMPARE(model.rowCount({}), 1);

    auto* pItem1 = model.addGroup("grpInRoot1", nullptr);
    QVERIFY(pItem1 != nullptr);
    QCOMPARE(model.rowCount({}), 2);

    auto* pItem2 = model.addPath("nonGrpRoot1", "path/to/nonGrpRoot1", QModelIndex{});
    QVERIFY(pItem2 != nullptr);
    QCOMPARE(model.rowCount({}), 3);

    QCOMPARE(pItem2->rowCount(), 0);
    QModelIndex item2Index{model.index(2, 0, {})};
    QCOMPARE(model.addGroup("nonGrpRoot1_child_group", item2Index), nullptr);  // cannot add anything under non group
    QCOMPARE(model.addPath("nonGrpRoot1_childPath", "path/to/nonGrpRoot1_childPath", item2Index), nullptr);
    QCOMPARE(pItem2->rowCount(), 0);
  }

  void setDatas_ok() {  //
    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(QByteArray{}), true);  // failed
    QCOMPARE(model.rowCount({}), 0);

    QVector<FavoriteItemData> elements;
    elements.push_back(FavoriteItemData{});                                         // invalid element 0
    elements.push_back(FavoriteItemData{"group"});                                  // valid element 1
    elements.push_back(FavoriteItemData{"non_group", "path/to/non_group_folder"});  // valid element 2
    QCOMPARE(model.setDatas(elements), true);
    QCOMPARE(model.rowCount({}), 2);

    auto r00 = model.index(0, 0, {});
    auto r10 = model.index(1, 0, {});

    QCOMPARE(model.data(r00, Qt::DisplayRole).toString(), "group");
    QCOMPARE(model.data(r00, FavoriteItemData::Role::FULL_PATH_ROLE).toString(), "");
    QCOMPARE(model.data(r10, Qt::DisplayRole).toString(), "non_group");
    QCOMPARE(model.data(r10, FavoriteItemData::Role::FULL_PATH_ROLE).toString(), "path/to/non_group_folder");

    QCOMPARE(model.convertDataToItem(FavoriteItemData{}), nullptr);
    QCOMPARE(model.convertItemToData(nullptr), (FavoriteItemData{}));
  }

  void removeParentIndexes_ok() {
    FavoriteItemData nonGroup0{"nonGroup0", "path/to/nonGroup0"};
    FavoriteItemData noChildGroup{"group0Child"};
    FavoriteItemData twoChildGroup{"group2Child"};
    twoChildGroup.children.push_back(FavoriteItemData{"group2Child/non_group_0", "path/to/group2Child/non_group_0"});
    twoChildGroup.children.push_back(FavoriteItemData{"group2Child/group_1"});
    FavoriteItemData threeChildGroup{"group3Child"};
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child0", "path/to/group3Child/child0"});
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child1"});
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child2"});
    FavoriteItemData nonGroup1{"nonGroup1", "path/to/nonGroup1"};

    QVector<FavoriteItemData> elements;
    elements.push_back(nonGroup0);
    elements.push_back(noChildGroup);
    elements.push_back(twoChildGroup);
    elements.push_back(threeChildGroup);
    elements.push_back(nonGroup1);
    QCOMPARE(elements.size(), 5);

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(elements), true);
    QCOMPARE(model.rowCount({}), 5);

    QCOMPARE(model.removeParentIndexes({}), 0);

    // remove all root groups and root non group (duplicate index exists. not crash)
    {
      QModelIndex rootIndex;  // will ignored

      QModelIndex r0 = model.index(0, 0, {});
      QModelIndex r1 = model.index(1, 0, {});
      QModelIndex r2 = model.index(2, 0, {});
      QModelIndex r3 = model.index(3, 0, {});
      QModelIndex r4 = model.index(4, 0, {});
      QCOMPARE(model.removeParentIndexes({rootIndex, r4, r1, r2, r0, r3, r4, r1, r2, r0, r3}), 5);
      QCOMPARE(model.rowCount({}), 0);
    }

    // remove root group and sub items
    {
      QCOMPARE(model.setDatas(elements), true);
      QCOMPARE(model.rowCount({}), 5);
      QModelIndex r0 = model.index(0, 0, {});
      QModelIndex r1 = model.index(1, 0, {});
      QModelIndex r2 = model.index(2, 0, {});
      QModelIndex r2_0 = model.index(0, 0, r2);
      QModelIndex r2_1 = model.index(1, 0, r2);
      QModelIndex r3 = model.index(3, 0, {});  // 保留
      QModelIndex r3_0 = model.index(0, 0, r3);
      QModelIndex r3_2 = model.index(2, 0, r3);

      QCOMPARE(model.removeParentIndexes({r0, r1, r2, r2_0, r2_1, r3_0, r3_2}), 5);  // r2, r2_0, r2_1中后两者无需单独删除, 只删除r2即可
      QCOMPARE(model.rowCount({}), 2);                                               // 剩下r3和r4
      QCOMPARE(model.index(0, 0, {}).data().toString(), "group3Child");
      QCOMPARE(model.index(1, 0, {}).data().toString(), "nonGroup1");
    }
  }

  void moveParentIndexesTo_ok() {
    FavoriteItemData nonGroup0{"nonGroup0", "path/to/nonGroup0"};
    FavoriteItemData noChildGroup{"group0Child"};
    FavoriteItemData twoChildGroup{"group2Child"};
    twoChildGroup.children.push_back(FavoriteItemData{"group2Child/non_group_0", "path/to/group2Child/non_group_0"});
    twoChildGroup.children.push_back(FavoriteItemData{"group2Child/group_1"});
    FavoriteItemData threeChildGroup{"group3Child"};
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child0", "path/to/group3Child/child0"});
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child1"});
    threeChildGroup.children.push_back(FavoriteItemData{"group3Child/child2"});
    FavoriteItemData nonGroup1{"nonGroup1", "path/to/nonGroup1"};

    QVector<FavoriteItemData> elements;
    elements.push_back(nonGroup0);
    elements.push_back(noChildGroup);
    elements.push_back(twoChildGroup);
    elements.push_back(threeChildGroup);
    elements.push_back(nonGroup1);
    QCOMPARE(elements.size(), 5);

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(elements), true);
    QCOMPARE(model.rowCount({}), 5);

    {
      QModelIndex rootIndex;                                  // will ignored
      QCOMPARE(model.moveParentIndexesTo({}, rootIndex), 0);  // no need

      QModelIndex nonGroup0RootIndex{model.index(0, 0, {})};  // cannot move to non-group
      QCOMPARE(model.moveParentIndexesTo({QModelIndex{}}, nonGroup0RootIndex), -1);

      QModelIndex twoChildGroupIndex{model.index(2, 0, {})};  // cannot move to itself
      QCOMPARE(model.moveParentIndexesTo({rootIndex, nonGroup0RootIndex, twoChildGroupIndex}, twoChildGroupIndex), -1);
    }

    // 根升序, 行号降序, move to root ok
    {
      QModelIndex rootIndex;
      QModelIndex r0{model.index(0, 0, rootIndex)};
      QModelIndex r1{model.index(1, 0, rootIndex)};
      QModelIndex r4{model.index(4, 0, rootIndex)};
      QCOMPARE(model.moveParentIndexesTo({r4, r0, r1, r0, r4}, rootIndex), 3);
      QCOMPARE(model.rowCount(), 5);

      // nonGroup0 group0Child group2Child group3Child nonGroup1
      // r0, r1, r4同根, 行号r4大; 排序后, r4, r1, r0按照顺序Append到尾部
      QCOMPARE(model.itemFromIndex(model.index(0, 0, rootIndex))->text(), "group2Child");
      QCOMPARE(model.itemFromIndex(model.index(1, 0, rootIndex))->text(), "group3Child");
      QCOMPARE(model.itemFromIndex(model.index(2, 0, rootIndex))->text(), "nonGroup1");
      QCOMPARE(model.itemFromIndex(model.index(3, 0, rootIndex))->text(), "group0Child");
      QCOMPARE(model.itemFromIndex(model.index(4, 0, rootIndex))->text(), "nonGroup0");
    }

    // 包含冗余, move to group ok
    {
      QCOMPARE(model.setDatas(elements), true);
      QCOMPARE(model.rowCount({}), 5);
      QModelIndex r0 = model.index(0, 0, {});
      QModelIndex r1 = model.index(1, 0, {});
      QModelIndex r2 = model.index(2, 0, {});
      QModelIndex r2_0 = model.index(0, 0, r2);
      QModelIndex r2_1 = model.index(1, 0, r2);
      QModelIndex r3 = model.index(3, 0, {});  // 保留
      QModelIndex r3_0 = model.index(0, 0, r3);
      QModelIndex r3_2 = model.index(2, 0, r3);

      QStandardItem* r0ParentAddress{model.invisibleRootItem()};
      QStandardItem* r2ParentAddress{model.invisibleRootItem()};
      QStandardItem* r3ParentAddress{model.itemFromIndex(r3)};
      QCOMPARE(r0ParentAddress, r2ParentAddress);

      // r0, r1, r2, r2_0, r2_1, r3, r3_0, r3_2
      // selected:[r0, r2, r3_0, r3_2] dest: r1
      // sort:
      // if address(r0 aka r2 parent) < address(r3_0 aka r3_2 parent): after sort: [r2, r0, r3_2, r3_0] => (r2, r0, r3_2, r3_0)
      // else: after sort: [r3_2, r3_0, r2, r0] => (r3_2, r3_0, r2, r0)

      QCOMPARE(model.moveParentIndexesTo({r0, r2, r2_0, r2_1, r3_0, r3_2}, r1), 4);  // r2, r2_0, r2_1中后两者无需单独移动, 只移动r2即可
      QCOMPARE(model.rowCount(), 3);

      const QModelIndex newR1Index{model.index(0, 0, {})};
      QCOMPARE(model.itemFromIndex(newR1Index)->text(), "group0Child");
      const QStandardItem* newR1Item{model.itemFromIndex(newR1Index)};
      QVERIFY(newR1Item != nullptr);
      QCOMPARE(newR1Item->rowCount(), 4);

      if (r0ParentAddress < r3ParentAddress) {
        QCOMPARE(model.index(0, 0, newR1Index).data(Qt::DisplayRole).toString(), "group2Child");
        QCOMPARE(model.index(1, 0, newR1Index).data(Qt::DisplayRole).toString(), "nonGroup0");
        QCOMPARE(model.index(2, 0, newR1Index).data(Qt::DisplayRole).toString(), "group3Child/child2");
        QCOMPARE(model.index(3, 0, newR1Index).data(Qt::DisplayRole).toString(), "group3Child/child0");
      } else {
        QCOMPARE(model.index(0, 0, newR1Index).data(Qt::DisplayRole).toString(), "group3Child/child2");
        QCOMPARE(model.index(1, 0, newR1Index).data(Qt::DisplayRole).toString(), "group3Child/child0");
        QCOMPARE(model.index(2, 0, newR1Index).data(Qt::DisplayRole).toString(), "group2Child");
        QCOMPARE(model.index(3, 0, newR1Index).data(Qt::DisplayRole).toString(), "nonGroup0");
      }

      const QModelIndex newR3Index{model.index(1, 0, {})};
      const QStandardItem* newR3Item{model.itemFromIndex(newR3Index)};
      QVERIFY(newR3Item != nullptr);
      QCOMPARE(newR3Item->text(), "group3Child");
      QCOMPARE(newR3Item->rowCount(), 1);

      const QModelIndex newR3_1Index{model.index(0, 0, newR3Index)};
      const QStandardItem* newR3_1Item{model.itemFromIndex(newR3_1Index)};
      QVERIFY(newR3_1Item != nullptr);
      QCOMPARE(newR3_1Item->text(), "group3Child/child1");

      QCOMPARE(model.itemFromIndex(model.index(2, 0, {}))->text(), "nonGroup1");
    }
  }

  void onRenameGroupName_ok() {
    FavoriteItemData nonGroup0{"nonGroup0", "path/to/nonGroup0"};
    FavoriteItemData noChildGroup{"group0Child"};
    QVector<FavoriteItemData> elements;
    elements.push_back(nonGroup0);
    elements.push_back(noChildGroup);
    QCOMPARE(elements.size(), 2);

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(elements), true);
    QCOMPARE(model.rowCount({}), 2);

    QCOMPARE(model.onRename({}, "Hallo"), false);                              // cannot rename root
    QCOMPARE(model.onRename(model.index(0, 0, {}), "nonGroup0"), true);        // name no change(here we don't check)
    QCOMPARE(model.onRename(model.index(0, 0, {}), "hallo nonGroup0"), true);  // can rename nongroup
    QCOMPARE(model.index(0, 0, {}).data().toString(), "hallo nonGroup0");
    QCOMPARE(model.itemFromIndex(model.index(0, 0, {}))->text(), "hallo nonGroup0");

    QCOMPARE(model.onRename(model.index(1, 0, {}), "group0Child"), true);        // name no change(here we don't check)
    QCOMPARE(model.onRename(model.index(1, 0, {}), "hallo group0Child"), true);  // can rename nongroup
    QCOMPARE(model.groupName(model.index(1, 0, {})), "hallo group0Child");
  }

  void drag_drop_internal_ok() {
    FavoriteItemData nonGroup0{"nonGroup0", "path/to/nonGroup0"};
    FavoriteItemData childGroup1{"groupChild1"};
    childGroup1.children.push_back(FavoriteItemData{"groupChild1_group0"});
    childGroup1.children.push_back(FavoriteItemData{"groupChild1_nonGroup1", "path/to/groupChild1_nonGroup1"});
    QVector<FavoriteItemData> elements;
    elements.push_back(nonGroup0);
    elements.push_back(childGroup1);
    QCOMPARE(elements.size(), 2);

    FavoritesTreeModel model{"BelongToFavoritesTreeViewInternalDragDrop", nullptr, false};
    QCOMPARE(model.setDatas(elements), true);
    QCOMPARE(model.rowCount({}), 2);

    // flags & canDropOn & isIndexValidAndDescendantOfValidAncestor
    QModelIndex rootIndex;
    QModelIndex nonGroup0Index{model.index(0, 0, rootIndex)};
    QModelIndex childGroup1Index{model.index(1, 0, rootIndex)};
    QModelIndex groupChild1_group0Index{model.index(0, 0, childGroup1Index)};
    QModelIndex groupChild1_nonGroup1{model.index(1, 0, childGroup1Index)};
    {
      // root: drop only
      QVERIFY(model.flags(rootIndex).testFlag(Qt::ItemFlag::ItemIsDropEnabled));
      QVERIFY(!model.flags(rootIndex).testFlag(Qt::ItemFlag::ItemIsDragEnabled));
      QVERIFY(model.canDropOn(rootIndex));

      // non-group: drag only
      QVERIFY(!model.flags(nonGroup0Index).testFlag(Qt::ItemFlag::ItemIsDropEnabled));
      QVERIFY(model.flags(nonGroup0Index).testFlag(Qt::ItemFlag::ItemIsDragEnabled));
      QVERIFY(!model.canDropOn(nonGroup0Index));

      // group: both drag and drop
      QVERIFY(model.flags(childGroup1Index).testFlag(Qt::ItemFlag::ItemIsDropEnabled));
      QVERIFY(model.flags(childGroup1Index).testFlag(Qt::ItemFlag::ItemIsDragEnabled));
      QVERIFY(model.canDropOn(childGroup1Index));

      QVERIFY(!FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor({}, rootIndex));
      QVERIFY(!FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(nonGroup0Index, rootIndex));
      QVERIFY(!FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(childGroup1Index, rootIndex));
      QVERIFY(!FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(childGroup1Index, nonGroup0Index));
      QVERIFY(FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(groupChild1_group0Index, childGroup1Index));
      QVERIFY(FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(groupChild1_nonGroup1, childGroup1Index));
    }

    {
      QCOMPARE(model.supportedDropActions(), (Qt::MoveAction | Qt::CopyAction));
      QCOMPARE(model.supportedDragActions(), (Qt::MoveAction | Qt::CopyAction));
      QCOMPARE(model.mimeTypes().size(), 1);
    }

    // mimeData;
    {
      // no selection
      QCOMPARE(model.mimeData({}), nullptr);
      // not first collum
      QCOMPARE(model.mimeData({rootIndex}), nullptr);
    }

    // canDropMimeData
    {
      QMimeData emptyData;
      // IgnoreAction永远为真
      QVERIFY(model.canDropMimeData(&emptyData, Qt::DropAction::IgnoreAction, -1, 0, rootIndex));
      // supportedDropActions不匹配
      QVERIFY(!model.canDropMimeData(&emptyData, Qt::DropAction::LinkAction, -1, 0, rootIndex));
      // 无MimeType
      QVERIFY(!model.canDropMimeData(&emptyData, Qt::DropAction::MoveAction, -1, 0, rootIndex));

      // 不检查内容, 只检查标志
      QMimeData validData;
      validData.setData(FavoritesTreeModel::MIME_TYPE, QByteArray{});
      // canDropOn为false
      QVERIFY(!model.canDropMimeData(&validData, Qt::DropAction::MoveAction, -1, 0, nonGroup0Index));

      // 空白处: 允许
      QVERIFY(model.canDropMimeData(&validData, Qt::DropAction::MoveAction, childGroup1Index.row(), childGroup1Index.column(), rootIndex));
      // group项上: 允许
      QVERIFY(model.canDropMimeData(&validData, Qt::DropAction::MoveAction, -1, 0, childGroup1Index));
    }

    // dropMimeData
    {
      QMimeData emptyData;
      // IgnoreAction永远为真
      QVERIFY(model.dropMimeData(&emptyData, Qt::DropAction::IgnoreAction, -1, 0, rootIndex));

      // 列号只能首列0
      QVERIFY(!model.dropMimeData(&emptyData, Qt::DropAction::MoveAction, -1, 1, rootIndex));

      // 没有MIME_TYPE
      QVERIFY(!model.dropMimeData(&emptyData, Qt::DropAction::MoveAction, -1, 0, rootIndex));
    }

    {  // mimeData, dropMimeData to 子结点 failed 避免嵌套结构(父不能往子里面放, 子能往放父里面)
      QMimeData* mimeData = model.mimeData({childGroup1Index});
      QVERIFY(mimeData != nullptr);
      QVERIFY(mimeData->hasFormat(FavoritesTreeModel::MIME_TYPE));
      QVERIFY(!model.dropMimeData(mimeData, Qt::DropAction::MoveAction, -1, 0, groupChild1_group0Index));
      QCOMPARE(model.rowCount(), 2);
    }

    {  // mimeData, dropMimeData to itself failed
      QMimeData* mimeData = model.mimeData({childGroup1Index});
      QVERIFY(mimeData != nullptr);
      QVERIFY(mimeData->hasFormat(FavoritesTreeModel::MIME_TYPE));
      QVERIFY(!model.dropMimeData(mimeData, Qt::DropAction::MoveAction, -1, 0, childGroup1Index));
      QCOMPARE(model.rowCount(), 2);
    }

    // mimeData, dropMimeData ok
    {
      QMimeData* mimeData = model.mimeData({nonGroup0Index, groupChild1_group0Index, groupChild1_nonGroup1});
      QVERIFY(mimeData != nullptr);
      QVERIFY(mimeData->hasFormat(FavoritesTreeModel::MIME_TYPE));

      QByteArray rawNodeToRootPathData = mimeData->data(FavoritesTreeModel::MIME_TYPE);
      QCOMPARE(rawNodeToRootPathData.isEmpty(), false);
      QDataStream readStream(rawNodeToRootPathData);
      auto version = FavoritesTreeModel::VERSION;
      readStream >> version;
      QCOMPARE(version, FavoritesTreeModel::VERSION);

      QList<int> nonGroup0IndexNode2Root;
      QList<int> groupChild1_group0IndexNode2Root;
      QList<int> groupChild1_nonGroup1Node2Root;
      readStream >> nonGroup0IndexNode2Root >> groupChild1_group0IndexNode2Root >> groupChild1_nonGroup1Node2Root;
      QCOMPARE(readStream.status(), QDataStream::Status::Ok);
      QCOMPARE(nonGroup0IndexNode2Root, (QList<int>{0}));
      QCOMPARE(groupChild1_group0IndexNode2Root, (QList<int>{0, 1}));
      QCOMPARE(groupChild1_nonGroup1Node2Root, (QList<int>{1, 1}));

      // 释放在空白root
      // r0, r1, r10, r11
      // selected:[r0, r10, r11] to rootPath
      // sort:
      // if address(r0 parent) < address(r10 parent): [r0, r11, r10] => r1 (r0, r11, r10)
      // else: [r11, r10, r0] => r1 (r11, r10, r0)
      const QStandardItem* r0ParentAddress = model.invisibleRootItem();
      const QStandardItem* r10ParentAddress = model.itemFromIndex(childGroup1Index);
      QCOMPARE(nonGroup0Index.data().toString(), "nonGroup0");
      QCOMPARE(childGroup1Index.data().toString(), "groupChild1");
      QCOMPARE(model.rowCount(), 2);
      QVERIFY(model.dropMimeData(mimeData, Qt::DropAction::MoveAction, -1, 0, rootIndex));
      QCOMPARE(model.rowCount(), 4);
      QCOMPARE(model.index(0, 0, rootIndex).data().toString(), "groupChild1");
      if (r0ParentAddress < r10ParentAddress) {
        QCOMPARE(model.index(1, 0, rootIndex).data().toString(), "nonGroup0");
        QCOMPARE(model.index(2, 0, rootIndex).data().toString(), "groupChild1_nonGroup1");
        QCOMPARE(model.index(3, 0, rootIndex).data().toString(), "groupChild1_group0");
      } else {
        QCOMPARE(model.index(1, 0, rootIndex).data().toString(), "groupChild1_nonGroup1");
        QCOMPARE(model.index(2, 0, rootIndex).data().toString(), "groupChild1_group0");
        QCOMPARE(model.index(3, 0, rootIndex).data().toString(), "nonGroup0");
      }
    }
  }

  void drag_drop_external_ok() {
    FavoritesTreeModel model{"BelongToFavoritesTreeViewExternalDragDrop", nullptr, false};
    QCOMPARE(model.setDatas(QByteArray{}), true);
    QCOMPARE(model.rowCount(), 0);

    QString urlPath{__FILE__};
    QString expectParentPath{QFileInfo(urlPath).absolutePath()};

    QList<QUrl> urls;
    urls.push_back(QUrl());  // non
    urls.push_back(QUrl::fromLocalFile(SystemPath::HOME_PATH()));
    urls.push_back(QUrl::fromLocalFile(SystemPath::STARRED_PATH()));
    urls.push_back(QUrl::fromLocalFile(urlPath));  // file
    QMimeData mimeData;
    mimeData.setUrls(urls);

    QModelIndex rootIndex;
    QVERIFY(model.canDropMimeData(&mimeData, Qt::DropAction::MoveAction, -1, 0, rootIndex));
    QVERIFY(model.dropMimeData(&mimeData, Qt::DropAction::MoveAction, -1, 0, rootIndex));

    QCOMPARE(model.index(0, 0, rootIndex).data(FavoriteItemData::FULL_PATH_ROLE).toString(), SystemPath::HOME_PATH());
    QCOMPARE(model.index(1, 0, rootIndex).data(FavoriteItemData::FULL_PATH_ROLE).toString(), SystemPath::STARRED_PATH());
    QCOMPARE(model.index(2, 0, rootIndex).data(FavoriteItemData::FULL_PATH_ROLE).toString(), expectParentPath);

    QCOMPARE(model.rowCount(), 3);
  }
};

#include "FavoritesTreeModelTest.moc"
REGISTER_TEST(FavoritesTreeModelTest, false)
