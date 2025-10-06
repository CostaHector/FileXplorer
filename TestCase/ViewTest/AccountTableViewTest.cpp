#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountTableView.h"
#include "EndToExposePrivateMember.h"
#include <QHeaderView>

#include "PwdTableEditActions.h"
#include "SimpleAES.h"
#include "TDir.h"

class AccountTableViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QString mEncCsvFilePath{tDir.itemPath("accounts_test.csv")};
  QString mPlainCsvFilePath{tDir.itemPath("exportedPlainAccounts_test.csv")};

  void prepareTestData(AccountStorage& storage) {
    QVector<AccountInfo> initialAccounts{
        {"Email", "Henry", "Henry@example.com", "pass123", "Additional info"},                 //
        {"Social Media", "Facebook", "fb_user", "fb_pass", "Notes with, comma\nand newline"},  //
        {"Bank", "Chris", "bank_user_chris", "bank_pass", ""},                                 //
    };
    storage.mAccounts = initialAccounts;
  }

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
    AccountTableViewMock::clear();
  }

  void cleanupTestCase() {
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    AccountStorageMock::clear();
    AccountTableViewMock::clear();
  }

  void behavior_ok() {
    PwdTableEditActions& ins = GetTableEditActionsInst();

    // 创建视图和准备数据
    AccountStorage storage;
    prepareTestData(storage);
    storage.SaveAccounts(true);

    AccountTableView view;
    QVERIFY(view.IsLoadSucceed());
    // view.mPwdModel->mAccountsList = storage;
    {
      // 步骤1: 验证初始状态
      QCOMPARE(view.mPwdModel->rowCount(), 3);
      QCOMPARE(view.model()->data(view.model()->index(0, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Email");
      QCOMPARE(view.model()->data(view.model()->index(1, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Social Media");
      QCOMPARE(view.model()->data(view.model()->index(2, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Bank");
    }

    {
      // 步骤2: 插入新行
      QCOMPARE(view.mPwdModel->rowCount(), 3);
      view.verticalHeader()->setCurrentIndex({});  // vertical invalid index
      AccountTableViewMock::getIntFromUserInputMock() = std::pair<bool, int>{true, 1};
      emit ins.INSERT_A_ROW->triggered();
      QCOMPARE(view.mPwdModel->rowCount(), 3);
      emit ins.INSERT_ROWS->triggered();
      QCOMPARE(view.mPwdModel->rowCount(), 3);

      QModelIndex vHeaderIndex2 = view.verticalHeader()->model()->index(2, 0);  // 在第二行后插入一行
      view.verticalHeader()->setCurrentIndex(vHeaderIndex2);
      AccountTableViewMock::getIntFromUserInputMock() = std::pair<bool, int>{true, 1};
      emit ins.INSERT_ROWS->triggered();  // view.InsertNRows(1)

      QCOMPARE(view.mPwdModel->rowCount(), 4);
      QCOMPARE(view.model()->data(view.model()->index(0, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Email");
      QCOMPARE(view.model()->data(view.model()->index(1, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Social Media");
      QCOMPARE(view.model()->data(view.model()->index(2, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "");  // 新插入的空行
      QCOMPARE(view.model()->data(view.model()->index(3, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Bank");
    }

    {
      // 步骤3: 编辑新行
      QModelIndex newRowIndex = view.model()->index(2, PwdPublicVariable::PwdTypeE::TYPE);
      view.model()->setData(newRowIndex, "Game");
      QCOMPARE(view.model()->data(newRowIndex).toString(), "Game");
    }

    {
      // 步骤4: 追加行
      AccountTableViewMock::getIntFromUserInputMock() = std::pair<bool, int>{true, 2};
      emit ins.APPEND_ROWS->triggered();  // view.AppendNRows(2)
      QCOMPARE(view.mPwdModel->rowCount(), 4 + 2);
    }

    {
      // 步骤5: 删除行
      // has nothing selected
      view.selectionModel()->clear();
      AccountTableViewMock::cfmRemoveSelectedRowMock() = true;
      QCOMPARE(view.RemoveSelectedRows(), 0);

      // 1st/3rd selected, but cancel
      AccountTableViewMock::cfmRemoveSelectedRowMock() = false;
      view.selectionModel()->select(view.model()->index(0, PwdPublicVariable::PwdTypeE::INDEX),
                                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
      view.selectionModel()->select(view.model()->index(2, PwdPublicVariable::PwdTypeE::INDEX),
                                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
      QCOMPARE(view.RemoveSelectedRows(), 0);

      // 1st/3rd selected, and accept
      AccountTableViewMock::cfmRemoveSelectedRowMock() = true;
      QCOMPARE(view.RemoveSelectedRows(), 2);
      QCOMPARE(view.mPwdModel->rowCount(), 4);
      QCOMPARE(view.model()->data(view.model()->index(0, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Social Media");
      QCOMPARE(view.model()->data(view.model()->index(1, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "Bank");
      QCOMPARE(view.model()->data(view.model()->index(2, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "");
      QCOMPARE(view.model()->data(view.model()->index(3, PwdPublicVariable::PwdTypeE::TYPE)).toString(), "");
    }

    {
      // filter
      QCOMPARE(view.mPwdModel->rowCount(), 4);
      QCOMPARE(view.mSortProxyModel->rowCount(), 4);
      view.SetFilter("Bank");
      QCOMPARE(view.mPwdModel->rowCount(), 4);
      QCOMPARE(view.mSortProxyModel->rowCount(), 1);
      const QModelIndex proInd = view.mSortProxyModel->index(0, PwdPublicVariable::PwdTypeE::TYPE);
      QCOMPARE(view.mSortProxyModel->data(proInd).toString(), "Bank");
      // clear filter
      view.SetFilter("");
      QCOMPARE(view.mPwdModel->rowCount(), 4);
      QCOMPARE(view.mSortProxyModel->rowCount(), 4);
    }

    {
      // export ok
      AccountTableViewMock::cfmExportPlainCSVMock() = false;
      QVERIFY(!view.ExportPlainCSV());
      QVERIFY(!tDir.exists(mPlainCsvFilePath));

      AccountTableViewMock::cfmExportPlainCSVMock() = true;
      QVERIFY(view.ExportPlainCSV());
      QVERIFY(tDir.exists(mPlainCsvFilePath));
    }

    {
      // 步骤9: 访问账户信息
      AccountInfo* info = nullptr;
      info = view.GetAccountInfoByCurrentIndex({});
      QVERIFY(info == nullptr);

      QModelIndex index = view.model()->index(1, PwdPublicVariable::PwdTypeE::INDEX);
      view.selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      info = view.GetAccountInfoByCurrentIndex(index);
      QVERIFY(info != nullptr);

      QCOMPARE(info->typeStr, "Bank");
      QCOMPARE(info->nameStr, "Chris");
    }

    {
      // 步骤10: 最终状态验证
      QCOMPARE(view.mPwdModel->rowCount(), 4);
    }

    {
      AccountTableViewMock::getIntFromUserInputMock() = std::pair<bool, int>{false, 10};
      QCOMPARE(view.GetRowsCountFromUserInput("Insert"), 0);
      AccountTableViewMock::getIntFromUserInputMock() = std::pair<bool, int>{true, 99};
      QCOMPARE(view.GetRowsCountFromUserInput("Append"), 99);
    }
  }
};

#include "AccountTableViewTest.moc"
REGISTER_TEST(AccountTableViewTest, false)
