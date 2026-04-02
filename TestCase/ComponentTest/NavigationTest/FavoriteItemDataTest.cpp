#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "FavoriteItemData.h"

class FavoriteItemDataTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void read_write_ok() {
    auto r0 = new MyTreeNode{TDataType{"group"}, {}, nullptr};

    auto r0_0 = new MyTreeNode{TDataType{"group/child0_g"}, {}, r0};
    auto r0_1 = new MyTreeNode{TDataType{"group/child1_g"}, {}, r0};
    auto r0_2 = new MyTreeNode{TDataType{"group/child2_l", "path/group/child2_l"}, {}, r0};
    auto r0_3 = new MyTreeNode{TDataType{"group/child3_l", "path/group/child3_l"}, {}, r0};

    auto r0_1_0 = new MyTreeNode{TDataType{"group/child1_g/group0"}, {}, r0_1};
    auto r0_1_1 = new MyTreeNode{TDataType{"group/child1_g/node1"}, {}, r0_1};
    r0_1->childs.push_back(r0_1_0);
    r0_1->childs.push_back(r0_1_1);

    r0->childs.push_back(r0_0);
    r0->childs.push_back(r0_1);
    r0->childs.push_back(r0_2);
    r0->childs.push_back(r0_3);
    std::unique_ptr<MyTreeNode> rootNode{r0};

    QByteArray ba;
    QDataStream writeDs(&ba, QIODevice::WriteOnly);
    writeDs << *rootNode;
    QCOMPARE(writeDs.status(), QDataStream::Status::Ok);
    QVERIFY(ba.size() > 0);

    std::unique_ptr<MyTreeNode> recoverRootNode{new MyTreeNode};
    QDataStream readDs(ba);
    readDs >> *recoverRootNode;
    QCOMPARE(readDs.status(), QDataStream::Status::Ok);

    QCOMPARE(*rootNode, *recoverRootNode);
  }
};

#include "FavoriteItemDataTest.moc"
REGISTER_TEST(FavoriteItemDataTest, false)
