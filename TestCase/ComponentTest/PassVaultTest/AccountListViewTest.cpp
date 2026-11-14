#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "AccountListView.h"

class AccountListViewTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void default_ok() {
    AccountTableModel model{"Path/To/An/Inexist/enc.csv", "RANDOM_AES_KEY"};
    QCOMPARE(model.rowCount(), 0); // 1 行
    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);

    AccountListView lv{&model, &proxyModel};

    QVERIFY(lv.itemDelegate() != nullptr);
    QVERIFY(lv.uniformItemSizes());
  }

};

#include "AccountListViewTest.moc"
REGISTER_TEST(AccountListViewTest, false)