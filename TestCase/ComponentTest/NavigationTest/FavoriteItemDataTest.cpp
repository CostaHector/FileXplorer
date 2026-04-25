#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "FavoriteItemData.h"
#include "EndToExposePrivateMember.h"
#include "MemoryKey.h"
#include "Configuration.h"

class FavoriteItemDataTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void initTestCase() { //
    QVERIFY(FavoriteItemData::COLUMN_COUNT > 0);
    constexpr int elementCntInHorArray{sizeof(FavoriteItemData::HOR_HEADER_TITLES) / sizeof(FavoriteItemData::HOR_HEADER_TITLES[0])};
    QCOMPARE(elementCntInHorArray, FavoriteItemData::COLUMN_COUNT);
    QCOMPARE(FavTreeNode::horizontalHeaderTitles(), FavoriteItemData::HOR_HEADER_TITLES);
  }

  void cleanupTestCase() { //
    Configuration().clear();
  }

  void sort_policy_get_save_ok() {
    FavoriteItemData::SaveInitialSortRole(FavoriteItemData::Role::FULL_PATH_ROLE);
    QCOMPARE(FavoriteItemData::GetInitialSortRole(), FavoriteItemData::Role::FULL_PATH_ROLE);
    // Set an invalid sort role value intentionally, expecting to return the default value
    Configuration().setValue(FavoritesNavigationKey::SORT_BY_ROLE.name, -1);
    QCOMPARE(FavoriteItemData::GetInitialSortRole(), FavoriteItemData::DEF_SORT_ROLE);

    FavoriteItemData::SaveSortOrderReverse(true);
    QCOMPARE(FavoriteItemData::GetInitialSortOrderReverse(), true);
  }

  void match_ok() {
    FavoriteItemData def;
    QCOMPARE(def.isGroup, true);
    QCOMPARE(def.name, "");
    QCOMPARE(def.fullPath, "");
    QCOMPARE(def.match("", Qt::CaseSensitive), true);
    QCOMPARE(def.match("", Qt::CaseInsensitive), true);
    QCOMPARE(def.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(def.match("randomText", Qt::CaseSensitive), false);
    QVERIFY(!def.isValid());

    FavoriteItemData group{"groupName"};
    QCOMPARE(group.isGroup, true);
    QCOMPARE(group.name, "groupName");
    QCOMPARE(group.fullPath, "");
    QCOMPARE(group.match("groupName", Qt::CaseSensitive), true);
    QCOMPARE(group.match("GROUPNAME", Qt::CaseSensitive), false);
    QCOMPARE(group.match("GROUPNAME", Qt::CaseInsensitive), true);
    QCOMPARE(group.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(group.match("randomText", Qt::CaseSensitive), false);
    QVERIFY(group.isValid());

    FavoriteItemData node{"nodeName", "path/nodeName"};
    QCOMPARE(node.isGroup, false);
    QCOMPARE(node.name, "nodeName");
    QCOMPARE(node.fullPath, "path/nodeName");
    QCOMPARE(node.match("nodeName", Qt::CaseSensitive), true);
    QCOMPARE(node.match("NODENAME", Qt::CaseSensitive), false);
    QCOMPARE(node.match("NODENAME", Qt::CaseInsensitive), true);
    QCOMPARE(node.match("path", Qt::CaseSensitive), true);
    QCOMPARE(node.match("PATH", Qt::CaseSensitive), false);
    QCOMPARE(node.match("PATH", Qt::CaseInsensitive), true);
    QCOMPARE(node.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(node.match("randomText", Qt::CaseSensitive), false);
    QVERIFY(node.isValid());

    QByteArray ba;
    QDataStream writeDs(&ba, QIODevice::WriteOnly);
    writeDs << node;
    QCOMPARE(writeDs.status(), QDataStream::Status::Ok);
    QVERIFY(ba.size() > 0);

    FavoriteItemData recoverNode;
    QDataStream readDs(ba);
    readDs >> recoverNode;

    QCOMPARE((recoverNode == node), true);
    QCOMPARE((recoverNode != node), false);

    QCOMPARE((group == node), false);
    QCOMPARE((group != node), true);

    QCOMPARE((def == node), false);
    QCOMPARE((def != node), true);
  }

  void FavTreeNode_serialize_and_deserialize_ok() {
    std::unique_ptr<FavTreeNode> r0 = FavTreeNode::NewTreeNodeRoot("FavoriteRoot");

    FavTreeNode* r0_0 = r0->appendRow(FavTreeNode::create(FavoriteItemData{"child0_g"}));
    FavTreeNode* r0_1 = r0->appendRow(FavTreeNode::create(FavoriteItemData{"child1_g"}));
    FavTreeNode* r0_2 = r0->appendRow(FavTreeNode::create(FavoriteItemData{"child2_l", "path/child2_l"}));
    FavTreeNode* r0_3 = r0->appendRow(FavTreeNode::create(FavoriteItemData{"child3_l", "path/child3_l"}));

    FavTreeNode* r0_1_0 = r0_1->appendRow(FavTreeNode::create(FavoriteItemData{"leaf0"}));
    FavTreeNode* r0_1_1 = r0_1->appendRow(FavTreeNode::create(FavoriteItemData{"node1", "path/node1"}));

    QCOMPARE(r0->childsCount(), 4);
    QCOMPARE(r0_0->childsCount(), 0);
    QCOMPARE(r0_1->childsCount(), 2);
    QCOMPARE(r0_2->childsCount(), 0);
    QCOMPARE(r0_3->childsCount(), 0);
    QCOMPARE(r0_1_0->childsCount(), 0);
    QCOMPARE(r0_1_1->childsCount(), 0);
    constexpr int totalNodeCount = 1 + 4 + 2;

    QCOMPARE((*r0 == *r0), true);
    QCOMPARE((*r0 != *r0), false);
    QCOMPARE((*r0 == *r0_0), false);
    QCOMPARE((*r0 != *r0_0), true);
    // r0_1_0 和 r0_3 无父子关系
    QVERIFY(!r0_1_0->isAncestorOf(r0_3));
    QVERIFY(!r0_3->isAncestorOf(r0_1_0));
    QVERIFY(!r0_1_0->isDescendantOf(r0_3));
    QVERIFY(!r0_3->isDescendantOf(r0_1_0));

    // r0_1_0 和 r0_1_1 无父子关系
    QVERIFY(!r0_1_0->isAncestorOf(r0_1_1));
    QVERIFY(!r0_1_1->isAncestorOf(r0_1_0));
    QVERIFY(!r0_1_0->isDescendantOf(r0_1_1));
    QVERIFY(!r0_1_1->isDescendantOf(r0_1_0));

    QVERIFY(r0_0->isDescendantOf(r0.get()));
    QVERIFY(r0_1->isDescendantOf(r0.get()));
    QVERIFY(r0_2->isDescendantOf(r0.get()));
    QVERIFY(r0_3->isDescendantOf(r0.get()));
    QVERIFY(r0_1_0->isDescendantOf(r0.get()));
    QVERIFY(r0_1_1->isDescendantOf(r0.get()));
    QVERIFY(r0_1_0->isDescendantOf(r0_1));
    QVERIFY(r0_1_1->isDescendantOf(r0_1));

    QVERIFY(r0->isAncestorOf(r0_0));
    QVERIFY(r0->isAncestorOf(r0_1));
    QVERIFY(r0->isAncestorOf(r0_2));
    QVERIFY(r0->isAncestorOf(r0_3));
    QVERIFY(r0->isAncestorOf(r0_1_0));
    QVERIFY(r0->isAncestorOf(r0_1_1));
    QVERIFY(!r0_0->isAncestorOf(r0_1_0));
    QVERIFY(!r0_0->isAncestorOf(r0_1_1));
    QVERIFY(r0_1->isAncestorOf(r0_1_0));
    QVERIFY(r0_1->isAncestorOf(r0_1_1));

    QCOMPARE(r0->GetConfigKey(), "FavoriteRoot");
    QCOMPARE(r0_0->GetConfigKey(), "FavoriteRoot/child0_g");
    QCOMPARE(r0_1->GetConfigKey(), "FavoriteRoot/child1_g");
    QCOMPARE(r0_2->GetConfigKey(), "FavoriteRoot/child2_l");
    QCOMPARE(r0_3->GetConfigKey(), "FavoriteRoot/child3_l");
    QCOMPARE(r0_1_0->GetConfigKey(), "FavoriteRoot/child1_g/leaf0");
    QCOMPARE(r0_1_1->GetConfigKey(), "FavoriteRoot/child1_g/node1");

    { // filterAccept
      QString searchText{"NotExistName"};
      QHash<const void*, bool> passCache;
      decltype(passCache) expectPassCache;
      QCOMPARE(r0->filterAccept(searchText, passCache), false);
      QCOMPARE(passCache.size(), totalNodeCount); // 遍历入口根, 无任何匹配, 所有节点过滤结果都已经缓存
      expectPassCache = decltype(passCache){
          {r0.get(), false}, //
          {r0_0, false},     //
          {r0_1, false},     //
          {r0_2, false},     //
          {r0_3, false},     //
          {r0_1_0, false},   //
          {r0_1_1, false},   //
      };
      QCOMPARE(passCache, expectPassCache);

      QCOMPARE(r0_1->filterAccept(searchText, passCache), false);
      QCOMPARE(r0_1_0->filterAccept(searchText, passCache), false);
      QCOMPARE(r0_1_1->filterAccept(searchText, passCache), false);
      QCOMPARE(passCache.size(), totalNodeCount); // 多次查询, 缓存词典不会增大

      {
        // 目标就在itself
        searchText = "node1";
        passCache.clear();
        QCOMPARE(r0_1_1->isCurrentNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1);
        expectPassCache = decltype(passCache){
            {r0_1_1, true}, //
        };
        QCOMPARE(passCache, expectPassCache);

        passCache.clear();
        QCOMPARE(r0_1_1->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1); // 先1个自己节点
        expectPassCache = decltype(passCache){
            {r0_1_1, true}, //
        };
        QCOMPARE(passCache, expectPassCache);
      }

      {
        // 目标在2个child中的后者
        searchText = "node1";
        passCache.clear();
        QCOMPARE(r0_1->isChildNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 2);
        expectPassCache = decltype(passCache){
            {r0_1_0, false}, //
            {r0_1_1, true},  //
        };
        QCOMPARE(passCache, expectPassCache);

        passCache.clear();
        QCOMPARE(r0_1->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1 + 2); // 先1个自己节点, 再2个子节点
        expectPassCache = decltype(passCache){
            {r0_1, false},   //
            {r0_1_0, false}, //
            {r0_1_1, true},  //
        };
        QCOMPARE(passCache, expectPassCache);
      }

      {
        // 目标就在parent
        searchText = "FavoriteRoot";
        passCache.clear();
        QCOMPARE(r0_1->isParentNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1);
        expectPassCache = decltype(passCache){
            {r0.get(), true}, //
        };
        QCOMPARE(passCache, expectPassCache);

        passCache.clear();
        QCOMPARE(r0_1_1->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1 + 0 + 2); // 先1个自己节点, 再0个子节点, 再2次父节点
        expectPassCache = decltype(passCache){
            {r0.get(), true}, //
            {r0_1, false},    //
            {r0_1_1, false},  //
        };
        QCOMPARE(passCache, expectPassCache);
      }
    }

    QByteArray ba;
    QDataStream writeDs(&ba, QIODevice::WriteOnly);
    writeDs << *r0;
    QCOMPARE(writeDs.status(), QDataStream::Status::Ok);
    QVERIFY(ba.size() > 0);

    std::unique_ptr<FavTreeNode> recoverr0{FavTreeNode::NewTreeNodeRoot()};
    QDataStream readDs(ba);
    readDs >> *recoverr0;
    QCOMPARE(readDs.status(), QDataStream::Status::Ok);

    QCOMPARE((*r0 == *recoverr0), true);
    QCOMPARE((*r0 != *recoverr0), false);
  }
};

#include "FavoriteItemDataTest.moc"
REGISTER_TEST(FavoriteItemDataTest, false)
