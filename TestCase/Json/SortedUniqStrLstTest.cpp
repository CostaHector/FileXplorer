#include <QtTest>
#include <QCoreApplication>
#include "TestCase/pub/MyTestSuite.h"
#include "Tools/Json/SortedUniqStrLst.h"

class SortedUniqStrLstTest : public MyTestSuite {
  Q_OBJECT
 public:
  SortedUniqStrLstTest() : MyTestSuite{false} {}
 private slots:
  void test_contruct() {
    SortedUniqStrLst lst0;
    QCOMPARE(lst0.isEmpty(), true);
    QCOMPARE(lst0.count(), 0);

    SortedUniqStrLst lst1Element{"A"};
    SortedUniqStrLst lst1ElementAlso{QStringList{"A"}};
    QCOMPARE(lst1Element.isEmpty(), false);
    QCOMPARE(lst1Element.count(), 1);
    QCOMPARE(lst1Element.join(), "A");
    QCOMPARE(lst1Element.toSortedList(), (QStringList{"A"}));

    SortedUniqStrLst lst1{"C,B,A,D"};
    const QStringList expectLst{"A", "B", "C", "D"};
    QCOMPARE(lst1.join(), "A,B,C,D");
    QCOMPARE(lst1.toSortedList(), expectLst);
    QCOMPARE(lst1.count(), 4);
    QCOMPARE(lst1.isEmpty(), false);
    SortedUniqStrLst lst1Also{QStringList{"D", "C", "B", "A"}};
    QCOMPARE(lst1, lst1Also);

    SortedUniqStrLst lst3{"A,C,B"};
    QVERIFY(lst3 != lst1);
    QCOMPARE(lst3.count(), 3);
    QCOMPARE(lst3.isEmpty(), false);
    QCOMPARE(lst3.join(), "A,B,C");
    QCOMPARE(lst3.toSortedList(), (QStringList{"A", "B", "C"}));
    SortedUniqStrLst lst3Also{QStringList{"B", "A", "C"}};
    QCOMPARE(lst3, lst3Also);

    QStringList expect10{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    SortedUniqStrLst lst10{"0,1,2,3,4,5,9,8,7,6"};
    SortedUniqStrLst lst10Also{expect10};
    QCOMPARE(lst10.count(), 10);
    QCOMPARE(lst10.isEmpty(), false);
    QCOMPARE(lst10.join(), "0,1,2,3,4,5,6,7,8,9");
    QCOMPARE(lst10.toSortedList(), expect10);
    QCOMPARE(lst10, lst10Also);
  }

  void test_remove() {
    QStringList expect3{"0", "3", "1", "8", "7", "5"};
    SortedUniqStrLst lst3{expect3};
    QCOMPARE(lst3.join(), "0,1,3,5,7,8");
    QCOMPARE(lst3.remove("9"), false);
    QCOMPARE(lst3.join(), "0,1,3,5,7,8");
    QCOMPARE(lst3.remove("3"), true);
    QCOMPARE(lst3.join(), "0,1,5,7,8");
    QCOMPARE(lst3.remove("0"), true);
    QCOMPARE(lst3.join(), "1,5,7,8");
    QCOMPARE(lst3.remove("1"), true);
    QCOMPARE(lst3.join(), "5,7,8");
    QCOMPARE(lst3.remove("5"), true);
    QCOMPARE(lst3.join(), "7,8");
    QCOMPARE(lst3.remove("7"), true);
    QCOMPARE(lst3.join(), "8");
    QCOMPARE(lst3.remove("8"), true);
    QCOMPARE(lst3.join(), "");
    QCOMPARE(lst3.remove("1"), false);
    QCOMPARE(lst3.join(), "");
  }

  void test_clear() {
    SortedUniqStrLst lst3{"0,99,100,88,77,55,11,40"};
    QCOMPARE(lst3.isEmpty(), false);
    lst3.clear();
    QCOMPARE(lst3.join(), "");
    QCOMPARE(lst3.isEmpty(), true);
  }
};

#include "SortedUniqStrLstTest.moc"
SortedUniqStrLstTest gSortedUniqStrLstTest;
