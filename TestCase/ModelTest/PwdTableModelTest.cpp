#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "PwdTableModel.h"
#include "EndToExposePrivateMember.h"
#include "SimpleAES.h"
#include "TDir.h"

class PwdTableModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};
  QString mPlainCsvFilePath{tDir.itemPath("exportedPlainAccounts_test.csv")};
 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());  //
    QVERIFY(!tDir.exists("accounts_test.csv"));
    QVERIFY(!tDir.exists("exportedPlainAccounts_test.csv"));
    AccountStorageMock::clear();

    // register
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    AccountStorageMock::GetFullEncCsvFilePathMock() = mEncCsvFilePath;
    AccountStorageMock::GetFullPlainCsvFilePathMock() = mPlainCsvFilePath;
    QVERIFY(AccountStorage::IsAccountCSVFileInExistOrEmpty());

    // 初始化 AES 密钥
    SimpleAES::setKey("TestKey1234567890");  // 16字符密钥
  }

  void cleanupTestCase() {
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    AccountStorageMock::clear();
  }

  void defaultConstructor_ok() {
    using namespace PwdPublicVariable;
    // call following member function should not crash down
    QVERIFY(!tDir.exists("accounts_test.csv"));
    PwdTableModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), PwdPublicVariable::PWD_TABLE_HEADERS_COUNT);

    QModelIndex invalidIndex;
    QVERIFY(!model.data(invalidIndex).isValid());

    /*"Index", "Type", "Name", "Account"*/
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "INDEX");
    QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "TYPE");
    QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), "NAME");
    QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), "ACCOUNT");
    // section out of range display role, return value directly
    QCOMPARE(model.headerData(-4, Qt::Horizontal, Qt::DisplayRole).toInt(), -4 + 1);
    QCOMPARE(model.headerData(4, Qt::Horizontal, Qt::DisplayRole).toInt(), 4 + 1);
    QCOMPARE(model.headerData(-4, Qt::Vertical, Qt::DisplayRole).toInt(), -4 + 1);
    QCOMPARE(model.headerData(4, Qt::Vertical, Qt::DisplayRole).toInt(), 4 + 1);

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::TextAlignmentRole).toInt(), static_cast<int>(Qt::AlignRight));
    QVERIFY(model.headerData(0, Qt::Horizontal, Qt::ItemDataRole::ForegroundRole).isNull());

    QVERIFY(!model.IsDirty());
    QString msg;
    QCOMPARE(model.onSave(&msg), PwdPublicVariable::SAVE_RESULT::SKIP);
  }

  void behavior_ok() {
    using namespace PwdPublicVariable;
    AccountStorage storage;
    QVector<AccountInfo> initialAccounts{
        {"Email", "Personal", "user@example.com", "pass123", "Additional info"},         //
        {"Social Media", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"},  //
        {"Bank", "Savings", "bank_user", "bank_pass", ""}                                //
    };
    storage.mAccounts = initialAccounts;
    QVERIFY(storage.SaveAccounts(true));
    QVERIFY(tDir.exists("accounts_test.csv"));

    PwdTableModel model;
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(), PwdPublicVariable::PWD_TABLE_HEADERS_COUNT);
    QVERIFY(model.GetLoadResult());  // 验证加载成功

    // index column, cannot edit, can select
    QVERIFY(model.flags(model.index(0, PwdTypeE::INDEX)).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(!model.flags(model.index(0, PwdTypeE::INDEX)).testFlag(Qt::ItemFlag::ItemIsEditable));
    QVERIFY(model.flags(model.index(0, PwdTypeE::NAME)).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(model.flags(model.index(0, PwdTypeE::NAME)).testFlag(Qt::ItemFlag::ItemIsEditable));

    // 3. 测试数据访问
    QCOMPARE(model.data(model.index(0, PwdTypeE::INDEX)).toInt(), 0);
    QCOMPARE(model.data(model.index(0, PwdTypeE::TYPE)).toString(), "Email");
    QCOMPARE(model.data(model.index(0, PwdTypeE::NAME)).toString(), "Personal");
    QCOMPARE(model.data(model.index(0, PwdTypeE::ACCOUNT)).toString(), "user@example.com");

    QCOMPARE(model.data(model.index(1, PwdTypeE::TYPE)).toString(), "Social Media");
    QCOMPARE(model.data(model.index(1, PwdTypeE::TYPE), Qt::TextAlignmentRole).toInt(), int(Qt::AlignLeft));
    QVERIFY(model.data(model.index(1, PwdTypeE::TYPE), Qt::ForegroundRole).isNull());

    // 4. 测试数据修改
    QVERIFY(!model.setData(model.index(0, PwdTypeE::INDEX), 999));
    QCOMPARE(model.data(model.index(0, PwdTypeE::INDEX)).toInt(), 0);
    QVERIFY(model.setData(model.index(0, PwdTypeE::TYPE), "Social Media"));
    QCOMPARE(model.data(model.index(0, PwdTypeE::TYPE)).toString(), "Social Media");
    QVERIFY(model.setData(model.index(0, PwdTypeE::NAME), "New Personal"));
    QCOMPARE(model.data(model.index(0, PwdTypeE::NAME)).toString(), "New Personal");
    QVERIFY(model.setData(model.index(0, PwdTypeE::ACCOUNT), "New Work Email"));
    QCOMPARE(model.data(model.index(0, PwdTypeE::ACCOUNT)).toString(), "New Work Email");
    QVERIFY(model.IsDirty());

    // 5. 测试删除行
    QCOMPARE(model.RemoveIndexes({}), 0);
    QCOMPARE(model.RemoveIndexes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}), -1);  // there is no 12 rows at all
    QCOMPARE(model.RemoveIndexes(std::set<int>{1}), 1); // the 1th, 3th is left
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0, PwdTypeE::TYPE)).toString(), "Social Media");
    QCOMPARE(model.data(model.index(1, PwdTypeE::TYPE)).toString(), "Bank");

    // 6. 测试插入/删除行
    QCOMPARE(model.InsertNRows(1, 0), 0); // 0 rows inserted
    QCOMPARE(model.InsertNRows(-1, 1), 1); // 1 rows inserted at first
    QCOMPARE(model.InsertNRows(999, 1), 1); // 1 rows inserted at back
    QCOMPARE(model.rowCount(), 2 + 2);
    QCOMPARE(model.RemoveIndexes(std::set<int>{0, 3}), 2);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.InsertNRows(1, 1), 1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.data(model.index(2, 1)).toString(), "Bank");  // 原第二行变为第三行

    // 7. 测试添加账户记录
    QCOMPARE(model.AppendAccountRecords({}), 0);
    QCOMPARE(model.rowCount(), 3 + 0);
    QVector<AccountInfo> newAccounts{{"Game", "Steam", "steam_user", "steam_pass", "Gaming account"}};
    QCOMPARE(model.AppendAccountRecords(newAccounts), 1);
    QCOMPARE(model.rowCount(), 3 + 1);
    QCOMPARE(model.data(model.index(3, PwdTypeE::TYPE)).toString(), "Game");

    // 8. 测试保存操作
    QString saveDetail;
    PwdPublicVariable::SAVE_RESULT saveResult = model.onSave(&saveDetail);
    QCOMPARE(saveResult, PwdPublicVariable::SAVE_RESULT::OK);
    QVERIFY(!saveDetail.isEmpty());
    QVERIFY(!model.IsDirty());  // 保存后脏标志应清除

    // 9. 测试导出为明文CSV
    QVERIFY(model.ExportToPlainCSV());
    QVERIFY(tDir.exists("exportedPlainAccounts_test.csv"));

    // 10. 测试代理模型过滤
    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);
    proxyModel.BindAccountsList(model.mAccountsList);

    // 设置过滤关键词
    proxyModel.setFilterFixedString("Steam");
    QCOMPARE(proxyModel.rowCount(), 1);  // 应只匹配一行
    QModelIndex proxyIndex = proxyModel.index(0, PwdTypeE::TYPE);
    QCOMPARE(proxyModel.data(proxyIndex).toString(), "Game");

    // 11. 测试行数据访问
    AccountInfo* rowData = model.rowDataAt(3);
    QVERIFY(rowData != nullptr);
    QCOMPARE(rowData->typeStr, "Game");

    // 12. 测试无效行数据访问
    AccountInfo* invalidRowData = model.rowDataAt(100);
    QVERIFY(invalidRowData == nullptr);

    // 13. 测试图标显示
    QModelIndex iconIndex = model.index(3, PwdTypeE::TYPE);
    QVariant iconVariant = model.data(iconIndex, Qt::DecorationRole);
    QVERIFY(iconVariant.isValid());
  }
};

#include "PwdTableModelTest.moc"
REGISTER_TEST(PwdTableModelTest, false)
