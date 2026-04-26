#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "QAbstractTreeModelPub.h"
#include "FavoritesTreeModel.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"

class QAbstractTreeModelPubTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void data_retrieve_ok() {
    Configuration().clear();
    FavoritesTreeModel model{"FavoritesDataRetrieveOk", nullptr, false};
    QCOMPARE(model.rowCount({}), 0);
    QCOMPARE(model.columnCount({}), FavoriteItemData::COLUMN_COUNT);

    auto emptyRootNode = FavTreeNode::NewTreeNodeRoot();
    model.setDatas(std::move(emptyRootNode));
    QCOMPARE(model.rowCount({}), 0);

    auto rootNode = FavTreeNode::NewTreeNodeRoot();
    auto r0_0 = rootNode->appendRow(new FavTreeNode{FavoriteItemData{"group/child0_g"}});
    auto r0_1 = rootNode->appendRow(new FavTreeNode{FavoriteItemData{"group/child1_g"}});
    auto r0_2 = rootNode->appendRow(new FavTreeNode{FavoriteItemData{"group/child2_l", "path/group/child2_l"}});
    auto r0_3 = rootNode->appendRow(new FavTreeNode{FavoriteItemData{"group/child3_l", "path/group/child3_l"}});
    r0_1->appendRow(new FavTreeNode{FavoriteItemData{"group/child1_g/group0"}});
    r0_1->appendRow(new FavTreeNode{FavoriteItemData{"group/child1_g/node1"}});

    QCOMPARE(*r0_0, *r0_0);
    QVERIFY(*r0_0 != *r0_1);

    QCOMPARE(rootNode->childsCount(), 4);
    model.setDatas(std::move(rootNode));
    QCOMPARE(model.rowCount({}), 4);

    FavTreeNode* rootItem = model.invisibleRootItem();

    QModelIndex rootIndex;
    QModelIndex index0 = model.index(0, 0, rootIndex);
    QModelIndex index1 = model.index(1, 0, rootIndex);
    QModelIndex index2 = model.index(2, 0, rootIndex);
    QModelIndex index3 = model.index(3, 0, rootIndex);

    FavTreeNode* item0 = model.itemFromIndex(index0);
    FavTreeNode* item1 = model.itemFromIndex(index1);
    FavTreeNode* item2 = model.itemFromIndex(index2);
    FavTreeNode* item3 = model.itemFromIndex(index3);

    QCOMPARE(model.indexFromItem(item0), index0);
    QCOMPARE(model.indexFromItem(item1), index1);
    QCOMPARE(model.indexFromItem(item2), index2);
    QCOMPARE(model.indexFromItem(item3), index3);

    QCOMPARE(item0->parent(), rootItem);
    QCOMPARE(item1->parent(), rootItem);
    QCOMPARE(item2->parent(), rootItem);
    QCOMPARE(item3->parent(), rootItem);

    QCOMPARE(index0.row(), 0);
    QCOMPARE(index1.row(), 1);
    QCOMPARE(index2.row(), 2);
    QCOMPARE(index3.row(), 3);

    QCOMPARE(model.rowCount(index0), 0);
    QCOMPARE(model.rowCount(index1), 2);
    QCOMPARE(model.rowCount(index2), 0);
    QCOMPARE(model.rowCount(index3), 0);

    QModelIndex index10 = model.index(0, 0, index1);
    QModelIndex index11 = model.index(1, 0, index1);
    QCOMPARE(model.rowCount(index10), 0);
    QCOMPARE(model.rowCount(index11), 0);

    QCOMPARE(model.data(index0, Qt::DisplayRole).toString(), "group/child0_g");
    QCOMPARE(model.data(index1, Qt::DisplayRole).toString(), "group/child1_g");
    QCOMPARE(model.data(index2, Qt::DisplayRole).toString(), "group/child2_l");
    QCOMPARE(model.data(index3, Qt::DisplayRole).toString(), "group/child3_l");

    QCOMPARE(model.data(index0, FavoriteItemData::DEF_NAME_TEXT_ROLE).toString(), "group/child0_g");
    QCOMPARE(model.data(index1, FavoriteItemData::DEF_NAME_TEXT_ROLE).toString(), "group/child1_g");
    QCOMPARE(model.data(index2, FavoriteItemData::DEF_NAME_TEXT_ROLE).toString(), "group/child2_l");
    QCOMPARE(model.data(index3, FavoriteItemData::DEF_NAME_TEXT_ROLE).toString(), "group/child3_l");

    QCOMPARE(model.data(index0, FavoriteItemData::FULL_PATH_ROLE).toString(), "");
    QCOMPARE(model.data(index1, FavoriteItemData::FULL_PATH_ROLE).toString(), "");
    QCOMPARE(model.data(index2, FavoriteItemData::FULL_PATH_ROLE).toString(), "path/group/child2_l");
    QCOMPARE(model.data(index3, FavoriteItemData::FULL_PATH_ROLE).toString(), "path/group/child3_l");

    QCOMPARE(model.data(index0, FavoriteItemData::IS_GROUP_ROLE).toBool(), true);
    QCOMPARE(model.data(index1, FavoriteItemData::IS_GROUP_ROLE).toBool(), true);
    QCOMPARE(model.data(index2, FavoriteItemData::IS_GROUP_ROLE).toBool(), false);
    QCOMPARE(model.data(index3, FavoriteItemData::IS_GROUP_ROLE).toBool(), false);

    QCOMPARE(model.data(index0, FavoriteItemData::LAST_ACCESS_ROLE).toInt(), 0);
    QCOMPARE(model.data(index1, FavoriteItemData::LAST_ACCESS_ROLE).toInt(), 0);
    QCOMPARE(model.data(index2, FavoriteItemData::LAST_ACCESS_ROLE).toInt(), 0);
    QCOMPARE(model.data(index3, FavoriteItemData::LAST_ACCESS_ROLE).toInt(), 0);

    QCOMPARE(model.data(index0, FavoriteItemData::ACCESS_COUNT_ROLE).toInt(), 0);
    QCOMPARE(model.data(index1, FavoriteItemData::ACCESS_COUNT_ROLE).toInt(), 0);
    QCOMPARE(model.data(index2, FavoriteItemData::ACCESS_COUNT_ROLE).toInt(), 0);
    QCOMPARE(model.data(index3, FavoriteItemData::ACCESS_COUNT_ROLE).toInt(), 0);
  }
};

#include "QAbstractTreeModelPubTest.moc"
REGISTER_TEST(QAbstractTreeModelPubTest, false)
