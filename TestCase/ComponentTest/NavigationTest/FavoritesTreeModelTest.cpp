#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FavoritesTreeModel.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"
#include "PublicVariable.h"

#include <QMimeData>

class FavoritesTreeModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {  //
    Configuration().remove("BelongToFavoritesTreeView/DATAS");
  }

  void default_construct_ok() {
    {  // initialCollections: 2 add group, 1 add path
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, true};
      QCOMPARE(model.rowCount({}), 3);  // count(root)=3
      model.saveToSettings();
      QCOMPARE(Configuration().contains("BelongToFavoritesTreeView/DATAS"), true);

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
      QCOMPARE(model.itemFromIndex(model.index(3, 0, {}))->val.name, "random name");
      QCOMPARE(model.isGroup(model.index(3, 0, {})), false);
      QCOMPARE(model.filePath(model.index(3, 0, {})), "path/to/random name");
      // will call save to in destruction automatically
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView/DATAS"), true);

    {  // no initialCollections, vector<FavoriteItemData>.size() == 4
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.rowCount({}), 4);  // count(root)=3+1
      QCOMPARE(model.itemFromIndex(model.index(3, 0, {}))->val.name, "random name");
      QCOMPARE(model.isGroup(model.index(3, 0, {})), false);
      QCOMPARE(model.filePath(model.index(3, 0, {})), "path/to/random name");

      QVERIFY(model.setDatas(QByteArray{}));
      QCOMPARE(model.rowCount({}), 0);  // cleared
      // will call save to in destruction automatically
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView/DATAS"), true);

    {  // no initialCollections, vector<FavoriteItemData>.size() == 0
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.rowCount({}), 0);  // count(root)=0
      model.setThisTimeNotSave(true);
      Configuration().remove("BelongToFavoritesTreeView/DATAS");
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView/DATAS"), false);

    {  // no initialCollections, no key exist
      FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
      QCOMPARE(model.mNotSaveDatasThisTimeBeforeDestruct, false);
      QCOMPARE(model.rowCount({}), 0);  // count(root)=0
      model.setThisTimeNotSave(true);
    }
    QCOMPARE(Configuration().contains("BelongToFavoritesTreeView/DATAS"), false);
  }

  void addGroup_ok() {
    Configuration().remove("BelongToFavoritesTreeView/DATAS");
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

    auto r0 = FavTreeNode::NewTreeNodeRoot();
    r0->appendRow(new FavTreeNode{FavoriteItemData{"group"}});                                  // valid element 1
    r0->appendRow(new FavTreeNode{FavoriteItemData{"non_group", "path/to/non_group_folder"}});  // valid element 2

    QCOMPARE(model.setDatas(std::move(r0)), true);
    QCOMPARE(model.rowCount({}), 2);

    auto ind00 = model.index(0, 0, {});
    auto ind10 = model.index(1, 0, {});

    QCOMPARE(model.data(ind00, Qt::DisplayRole).toString(), "group");
    QCOMPARE(model.data(ind00, FavoriteItemData::Role::FULL_PATH_ROLE).toString(), "");
    QCOMPARE(model.data(ind10, Qt::DisplayRole).toString(), "non_group");
    QCOMPARE(model.data(ind10, FavoriteItemData::Role::FULL_PATH_ROLE).toString(), "path/to/non_group_folder");
  }

  void moveParentIndexesTo_sameParent_ok() {
    auto r = FavTreeNode::NewTreeNodeRoot();
    auto r0 = r->appendRow(new FavTreeNode{FavoriteItemData{"g0"}});  // ancestor same parent
    auto r1 = r->appendRow(new FavTreeNode{FavoriteItemData{"g1"}});
    auto r2 = r->appendRow(new FavTreeNode{FavoriteItemData{"n2", "path/n2"}});
    auto r00 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n0", "path/g0/n0"}});  // Descendant
    auto r01 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n1", "path/g0/n1"}});  // Descendant
    auto r02 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/g2"}});                // Descendant

    QVERIFY(r0->isDescendantOf(r.get()));
    QVERIFY(r0->isAncestorOf(r02));
    QVERIFY(r02->isDescendantOf(r0));
    QVERIFY(r0->isAncestorOf(r00));
    QVERIFY(!r00->isAncestorOf(r0));

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(std::move(r)), true);  // failed
    QModelIndex rIndex;
    QCOMPARE(model.rowCount(rIndex), 3);

    QModelIndex r0Index = model.index(0, 0, rIndex);
    QModelIndex r1Index = model.index(1, 0, rIndex);
    QModelIndex r2Index = model.index(2, 0, rIndex);
    QModelIndex r00Index = model.index(0, 0, r0Index);
    QModelIndex r01Index = model.index(1, 0, r0Index);
    QModelIndex r02Index = model.index(2, 0, r0Index);
    QCOMPARE(model.rowCount(r0Index), 3);

    // 0. nothing selected
    QCOMPARE(model.moveParentIndexesTo({}, r0Index), 0);

    // 1. cannot move to itself
    QCOMPARE(model.moveParentIndexesTo({r0Index}, r0Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r1Index}, r1Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r2Index}, r2Index), -1);

    // 2. cannot move to non-group
    QCOMPARE(model.moveParentIndexesTo({r0Index}, r2Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r02Index}, r00Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r02Index}, r01Index), -1);

    // 3. cannot move ancestor to its descendant
    QVERIFY(!r0->isDescendantOf(r02));
    QCOMPARE(model.moveParentIndexesTo({r0Index}, r02Index), -1);

    // can move r00 its r0
    QVERIFY(r00->isDescendantOf(r0));
    QCOMPARE(model.moveParentIndexesTo({r00Index}, r0Index), 1);
    QCOMPARE(model.rowCount(r0Index), 3);

    QCOMPARE(model.itemFromIndex(model.index(0, 0, r0Index)), r01);
    QCOMPARE(model.itemFromIndex(model.index(1, 0, r0Index)), r02);
    QCOMPARE(model.itemFromIndex(model.index(2, 0, r0Index)), r00);

    // can move r0 its r
    QCOMPARE(model.moveParentIndexesTo({r0Index}, rIndex), 1);
    QCOMPARE(model.itemFromIndex(model.index(0, 0, rIndex)), r1);
    QCOMPARE(model.itemFromIndex(model.index(1, 0, rIndex)), r2);
    QCOMPARE(model.itemFromIndex(model.index(2, 0, rIndex)), r0);
  }

  void moveParentIndexesTo_crossParent_ok() {
    auto r = FavTreeNode::NewTreeNodeRoot();
    auto r0 = r->appendRow(new FavTreeNode{FavoriteItemData{"g0"}});  // ancestor same parent
    auto r1 = r->appendRow(new FavTreeNode{FavoriteItemData{"g1"}});
    auto r2 = r->appendRow(new FavTreeNode{FavoriteItemData{"n2", "path/n2"}});
    auto r00 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n0", "path/g0/n0"}});  // Descendant
    auto r01 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n1", "path/g0/n1"}});  // Descendant
    auto r02 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/g2"}});                // Descendant

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(std::move(r)), true);  // failed
    QModelIndex rIndex;
    QCOMPARE(model.rowCount(rIndex), 3);

    QModelIndex r0Index = model.index(0, 0, rIndex);
    QModelIndex r1Index = model.index(1, 0, rIndex);
    QModelIndex r2Index = model.index(2, 0, rIndex);
    QModelIndex r00Index = model.index(0, 0, r0Index);
    QModelIndex r01Index = model.index(1, 0, r0Index);
    QModelIndex r02Index = model.index(2, 0, r0Index);
    QCOMPARE(model.rowCount(r0Index), 3);

    // cannot move to non-group
    QCOMPARE(model.moveParentIndexesTo({r00Index}, r2Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r01Index}, r2Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r02Index}, r2Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r1Index}, r00Index), -1);
    QCOMPARE(model.moveParentIndexesTo({r2Index}, r00Index), -1);

    QCOMPARE(model.rowCount(rIndex), 3);
    QCOMPARE(model.rowCount(r0Index), 3);
    QCOMPARE(model.rowCount(r1Index), 0);
    QCOMPARE(model.rowCount(r2Index), 0);
    QCOMPARE(model.moveParentIndexesTo({r00Index}, r1Index), 1);
    QCOMPARE(model.rowCount(rIndex), 3);
    QCOMPARE(model.rowCount(r0Index), 2);
    QCOMPARE(model.rowCount(r1Index), 1);
    QCOMPARE(model.rowCount(r2Index), 0);

    QCOMPARE(model.itemFromIndex(model.index(0, 0, r0Index)), r01);
    QCOMPARE(model.itemFromIndex(model.index(1, 0, r0Index)), r02);

    QCOMPARE(model.itemFromIndex(model.index(0, 0, r1Index)), r00);
  }

  void moveParentIndexesTo_Ancestor_ok() {
    auto r = FavTreeNode::NewTreeNodeRoot();
    auto r0 = r->appendRow(new FavTreeNode{FavoriteItemData{"g0"}});  // ancestor same parent
    auto r1 = r->appendRow(new FavTreeNode{FavoriteItemData{"g1"}});
    auto r2 = r->appendRow(new FavTreeNode{FavoriteItemData{"n2", "path/n2"}});
    auto r00 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n0", "path/g0/n0"}});  // Descendant
    auto r01 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/n1", "path/g0/n1"}});  // Descendant
    auto r02 = r0->appendRow(new FavTreeNode{FavoriteItemData{"g0/g2"}});                // Descendant

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(std::move(r)), true);  // failed
    QModelIndex rIndex;
    QModelIndex r0Index = model.index(0, 0, rIndex);
    QModelIndex r1Index = model.index(1, 0, rIndex);
    QModelIndex r2Index = model.index(2, 0, rIndex);
    QModelIndex r00Index = model.index(0, 0, r0Index);
    QModelIndex r01Index = model.index(1, 0, r0Index);
    QModelIndex r02Index = model.index(2, 0, r0Index);

    QCOMPARE(model.rowCount(rIndex), 3);
    QCOMPARE(model.rowCount(r0Index), 3);
    QCOMPARE(model.rowCount(r1Index), 0);
    QCOMPARE(model.rowCount(r2Index), 0);

    QCOMPARE(model.moveParentIndexesTo({r00Index}, rIndex), 1);

    QCOMPARE(model.rowCount(rIndex), 4);
    QCOMPARE(model.itemFromIndex(model.index(0, 0, rIndex)), r0);
    QCOMPARE(model.itemFromIndex(model.index(1, 0, rIndex)), r1);
    QCOMPARE(model.itemFromIndex(model.index(2, 0, rIndex)), r2);
    QCOMPARE(model.itemFromIndex(model.index(3, 0, rIndex)), r00);

    QCOMPARE(model.rowCount(r0Index), 2);
    QCOMPARE(model.itemFromIndex(model.index(0, 0, r0Index)), r01);
    QCOMPARE(model.itemFromIndex(model.index(1, 0, r0Index)), r02);

    QCOMPARE(model.rowCount(r1Index), 0);
    QCOMPARE(model.rowCount(r2Index), 0);
  }

  void removeParentIndexes_ok() {
    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    auto p = FavTreeNode::NewTreeNodeRoot();
    auto p0 = p->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup0", "path/to/nonGroup0"}});
    auto p1 = p->appendRow(new FavTreeNode{FavoriteItemData{"group0Child"}});
    auto p2 = p->appendRow(new FavTreeNode{FavoriteItemData{"group2Child"}});
    /*     */ p2->appendRow(new FavTreeNode{FavoriteItemData{"group2Child/non_group_0", "path/to/group2Child/non_group_0"}});
    /*     */ p2->appendRow(new FavTreeNode{FavoriteItemData{"group2Child/group_1"}});
    auto p3 = p->appendRow(new FavTreeNode{FavoriteItemData{"group3Child"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child0", "path/to/group3Child/child0"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child1"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child2"}});
    auto p4 = p->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup1", "path/to/nonGroup1"}});
    QCOMPARE(model.setDatas(std::move(p)), true);
    QCOMPARE(model.rowCount({}), 5);
    const QByteArray elements{model.toByteArray()};

    QCOMPARE(model.removeParentIndexes({}), 0);
    QCOMPARE(model.removeParentIndexes({QModelIndex{}}), -1); // 根节点, 不会被删除

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
    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    auto p = FavTreeNode::NewTreeNodeRoot();
    auto p0 = p->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup0", "path/to/nonGroup0"}});
    auto p1 = p->appendRow(new FavTreeNode{FavoriteItemData{"group0Child"}});
    auto p2 = p->appendRow(new FavTreeNode{FavoriteItemData{"group2Child"}});
    /*     */ p2->appendRow(new FavTreeNode{FavoriteItemData{"group2Child/non_group_0", "path/to/group2Child/non_group_0"}});
    /*     */ p2->appendRow(new FavTreeNode{FavoriteItemData{"group2Child/group_1"}});
    auto p3 = p->appendRow(new FavTreeNode{FavoriteItemData{"group3Child"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child0", "path/to/group3Child/child0"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child1"}});
    /*     */ p3->appendRow(new FavTreeNode{FavoriteItemData{"group3Child/child2"}});
    auto p4 = p->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup1", "path/to/nonGroup1"}});

    QCOMPARE(model.setDatas(std::move(p)), true);
    QCOMPARE(model.rowCount({}), 5);
    const QByteArray elements{model.toByteArray()};

    // 根升序, 行号降序, move to root ok
    {
      QModelIndex r0{model.index(0, 0, {})};
      QModelIndex r1{model.index(1, 0, {})};
      QModelIndex r4{model.index(4, 0, {})};
      QCOMPARE(model.moveParentIndexesTo({r4, r0, r1, r0, r4}, {}), 3);
      QCOMPARE(model.rowCount(), 5);

      // nonGroup0 group0Child group2Child group3Child nonGroup1
      // r0, r1, r4同根, 行号r4大; 排序后, r4, r1, r0按照顺序Append到尾部
      QCOMPARE(model.itemFromIndex(model.index(0, 0, {}))->val.name, "group2Child");
      QCOMPARE(model.itemFromIndex(model.index(1, 0, {}))->val.name, "group3Child");
      QCOMPARE(model.itemFromIndex(model.index(2, 0, {}))->val.name, "nonGroup1");
      QCOMPARE(model.itemFromIndex(model.index(3, 0, {}))->val.name, "group0Child");
      QCOMPARE(model.itemFromIndex(model.index(4, 0, {}))->val.name, "nonGroup0");
    }

    {
      QModelIndex rootIndex;                                         // will ignored
      QModelIndex nonGroup0RootIndex{model.index(0, 0, rootIndex)};  // cannot move to non-group
      QCOMPARE(model.moveParentIndexesTo({QModelIndex{}}, nonGroup0RootIndex), -1);

      QModelIndex twoChildGroupIndex{model.index(2, 0, rootIndex)};  // cannot move to itself
      QCOMPARE(model.moveParentIndexesTo({rootIndex, nonGroup0RootIndex, twoChildGroupIndex}, twoChildGroupIndex), -1);
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

      FavTreeNode* r0ParentAddress{model.invisibleRootItem()};
      FavTreeNode* r2ParentAddress{model.invisibleRootItem()};
      FavTreeNode* r3ParentAddress{model.itemFromIndex(r3)};
      QCOMPARE(r0ParentAddress, r2ParentAddress);

      // r0, r1, r2, r2_0, r2_1, r3, r3_0, r3_2
      // selected:[r0, r2, r3_0, r3_2] dest: r1
      // sort:
      // if address(r0 aka r2 parent) < address(r3_0 aka r3_2 parent): after sort: [r2, r0, r3_2, r3_0] => (r2, r0, r3_2, r3_0)
      // else: after sort: [r3_2, r3_0, r2, r0] => (r3_2, r3_0, r2, r0)

      QCOMPARE(model.moveParentIndexesTo({r0, r2, r2_0, r2_1, r3_0, r3_2}, r1), 4);  // r2, r2_0, r2_1中后两者无需单独移动, 只移动r2即可
      QCOMPARE(model.rowCount(), 3);

      const QModelIndex newR1Index{model.index(0, 0, {})};
      QCOMPARE(model.itemFromIndex(newR1Index)->val.name, "group0Child");
      const FavTreeNode* newR1Item{model.itemFromIndex(newR1Index)};
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
      const FavTreeNode* newR3Item{model.itemFromIndex(newR3Index)};
      QVERIFY(newR3Item != nullptr);
      QCOMPARE(newR3Item->val.name, "group3Child");
      QCOMPARE(newR3Item->rowCount(), 1);

      const QModelIndex newR3_1Index{model.index(0, 0, newR3Index)};
      const FavTreeNode* newR3_1Item{model.itemFromIndex(newR3_1Index)};
      QVERIFY(newR3_1Item != nullptr);
      QCOMPARE(newR3_1Item->val.name, "group3Child/child1");

      QCOMPARE(model.itemFromIndex(model.index(2, 0, {}))->val.name, "nonGroup1");
    }
  }

  void onRenameGroupName_ok() {
    auto r0 = FavTreeNode::NewTreeNodeRoot();
    r0->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup0", "path/to/nonGroup0"}});
    r0->appendRow(new FavTreeNode{FavoriteItemData{"group0Child"}});

    FavoritesTreeModel model{"BelongToFavoritesTreeView", nullptr, false};
    QCOMPARE(model.setDatas(std::move(r0)), true);
    QCOMPARE(model.rowCount({}), 2);

    QCOMPARE(model.onRename({}, "Hallo"), false);                              // cannot rename root
    QCOMPARE(model.onRename(model.index(0, 0, {}), "nonGroup0"), false);       // name no change
    QCOMPARE(model.onRename(model.index(0, 0, {}), "hallo nonGroup0"), true);  // can rename nongroup
    QCOMPARE(model.index(0, 0, {}).data().toString(), "hallo nonGroup0");
    QCOMPARE(model.itemFromIndex(model.index(0, 0, {}))->val.name, "hallo nonGroup0");

    QCOMPARE(model.onRename(model.index(1, 0, {}), "group0Child"), false);       // name no change
    QCOMPARE(model.onRename(model.index(1, 0, {}), "hallo group0Child"), true);  // can rename nongroup
    QCOMPARE(model.groupName(model.index(1, 0, {})), "hallo group0Child");
  }

  void drag_drop_internal_ok() {
    auto r0 = FavTreeNode::NewTreeNodeRoot();
    auto r00 = r0->appendRow(new FavTreeNode{FavoriteItemData{"nonGroup0", "path/to/nonGroup0"}});  // non-group
    auto r01 = r0->appendRow(new FavTreeNode{FavoriteItemData{"groupChild1"}});                     // group
    auto r010 = r01->appendRow(new FavTreeNode{FavoriteItemData{"groupChild1_group0"}});
    auto r011 = r01->appendRow(new FavTreeNode{FavoriteItemData{"groupChild1_nonGroup1", "path/to/groupChild1_nonGroup1"}});

    FavoritesTreeModel model{"BelongToFavoritesTreeViewInternalDragDrop", nullptr, false};
    QCOMPARE(model.setDatas(std::move(r0)), true);
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
      auto version = FavoritesTreeModel::GetVersion();
      version = 0;
      readStream >> version;
      QCOMPARE(version, FavoritesTreeModel::GetVersion());

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
      const FavTreeNode* r0ParentAddress = model.invisibleRootItem();
      const FavTreeNode* r10ParentAddress = model.itemFromIndex(childGroup1Index);
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

    QCOMPARE(model.index(0, 0, rootIndex).data(FavoriteItemData::Role::FULL_PATH_ROLE).toString(), SystemPath::HOME_PATH());
    QCOMPARE(model.index(1, 0, rootIndex).data(FavoriteItemData::Role::FULL_PATH_ROLE).toString(), SystemPath::STARRED_PATH());
    QCOMPARE(model.index(2, 0, rootIndex).data(FavoriteItemData::Role::FULL_PATH_ROLE).toString(), expectParentPath);

    QCOMPARE(model.rowCount(), 3);
  }
};

#include "FavoritesTreeModelTest.moc"
REGISTER_TEST(FavoritesTreeModelTest, false)
