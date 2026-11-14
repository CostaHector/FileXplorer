#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountTableView.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "SimpleAES.h"
#include <QHeaderView>
#include <QSignalSpy>

class AccountTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  QString mEncCsvFileName{"test_accounts.enc.csv"};
private slots:
  void signal_insertLinesReq_ok() {
    const QString AES_KEY{"0123456789012345"};
    QList<AccountInfo> accList {AccountInfo{"E-mail", "Gmail", "MichaelFassbender@gmail.com", "123456", ""}}; // 1个元素
    QString plainCSV = AccountFileHelper::GetExportCSVRecords(accList);
    QString encryptedCSV;
    QVERIFY(SimpleAES(AES_KEY).encrypt_GCM(plainCSV, encryptedCSV));
    QVERIFY(encryptedCSV.size() > 0);
    QVERIFY(mTDir.touch(mEncCsvFileName, encryptedCSV.toUtf8()));

    AccountTableModel model{mTDir.itemPath(mEncCsvFileName), AES_KEY};
    QCOMPARE(model.rowCount(), 1); // 1 行
    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);

    AccountTableView tv{&model, &proxyModel};
    QVERIFY(tv.itemDelegate() != nullptr);

    QHeaderView* pVerHeader = tv.verticalHeader();
    QVERIFY(pVerHeader != nullptr);
    QCOMPARE(pVerHeader->count(), 1);
    const QModelIndex firstInd = pVerHeader->model()->index(0, 0);
    pVerHeader->setCurrentIndex(firstInd);
    QCOMPARE(pVerHeader->currentIndex().row(), 0);
    QVERIFY(tv.mInsertAnRowAct != nullptr);

    QSignalSpy insertLinesReqSpy{&tv, &AccountTableView::insertLinesReq};
    QVERIFY(insertLinesReqSpy.isValid());

    // 有当前index
    tv.mInsertAnRowAct->trigger();
    QCOMPARE(insertLinesReqSpy.size(), 1);
    const QVariantList expectParms{0, 1};
    const QVariantList actualParms = insertLinesReqSpy.takeLast();
    QCOMPARE(actualParms, expectParms);

    // 没有当前index
    pVerHeader->setCurrentIndex(QModelIndex{});
    QVERIFY(pVerHeader->currentIndex().row() < 0);
    tv.mInsertAnRowAct->trigger();
    QCOMPARE(insertLinesReqSpy.size(), 0);
  }
};

#include "AccountTableViewTest.moc"
REGISTER_TEST(AccountTableViewTest, false)
