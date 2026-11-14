#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "PasswordBook.h"
#include "EndToExposePrivateMember.h"

#include "FileTool.h"
#include "TDir.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    class PasswordBookTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  QString mEncCsvFileName{"test_accounts.enc.csv"};

private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void fullpath_aes_key_changed_ok() {
    // 检查 isEnabled 属性
    PasswordBook pwdBook{"", "", PasswordBook::ViewType::LIST_VIEW, nullptr};
    QVERIFY(pwdBook.windowTitle().startsWith("Password Book"));

    // proxy要设置好了sourceModel
    QVERIFY(pwdBook.mPwdModel != nullptr);
    QVERIFY(pwdBook.mSortProxyModel != nullptr);
    QCOMPARE(pwdBook.mSortProxyModel->sourceModel(), pwdBook.mPwdModel);

    QVERIFY(pwdBook.mAccountFileAndAesWid != nullptr);
    QVERIFY(pwdBook.mLeftAreaToolBar != nullptr);
    QVERIFY(pwdBook.mMainWidget != nullptr);

    // 初始解析失败, 除了mAccountFileAndAesWid框外, 其他控件要禁用disable掉.
    {
      QVERIFY(!pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(!pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(!pwdBook.mMainWidget->isEnabled());
    }

    // onDecryptResultChanged 调用验证
    // 同时改好. 输入正确路径和密钥, 控件要启用enable掉.
    // disable->encFileFullPathAndAesKeyChanged->decryptedResultChanged->enabled
    {
      emit pwdBook.mAccountFileAndAesWid->encFileFullPathAndAesKeyChanged(mTDir.itemPath(mEncCsvFileName), "REGISTER_AES_KEY");
      QVERIFY(pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(pwdBook.mMainWidget->isEnabled());
    }

    // 改坏/改好 encFileFullPathChanged
    {
      emit pwdBook.mAccountFileAndAesWid->encFileFullPathChanged("Path/to/Inexist/PrePath");
      QVERIFY(!pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(!pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(!pwdBook.mMainWidget->isEnabled());

      emit pwdBook.mAccountFileAndAesWid->encFileFullPathChanged(mTDir.itemPath(mEncCsvFileName));
      QVERIFY(pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(pwdBook.mMainWidget->isEnabled());
    }

    // 改坏/改好 aesKeyChanged
    {
      emit pwdBook.mAccountFileAndAesWid->aesKeyChanged("");
      QVERIFY(!pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(!pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(!pwdBook.mMainWidget->isEnabled());

      emit pwdBook.mAccountFileAndAesWid->aesKeyChanged("REGISTER_AES_KEY");
      QVERIFY(pwdBook.mPwdModel->IsDecryptedOk());
      QVERIFY(pwdBook.mAccountFileAndAesWid->isEnabled());
      QVERIFY(pwdBook.mLeftAreaToolBar->isEnabled());
      QVERIFY(pwdBook.mMainWidget->isEnabled());
    }
  }

  void onAccountViewChanged_ok() {
    // 允许切换视图
    {
      // 初始为List
      PasswordBook pwdBook{"", "", -999, nullptr}; // viewType out of range. not crash
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::LIST_VIEW);
      QVERIFY(pwdBook.mMainWidget != nullptr);
      QVERIFY(pwdBook.mAccountListView != nullptr);  // lazy launch
      QVERIFY(pwdBook.mAccountTableView == nullptr); // lazy launch
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountListView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 0);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::LIST_VIEW], 0);

      QVERIFY(pwdBook.mAccountViewSwitcher != nullptr);
      QCOMPARE(pwdBook.mAccountViewSwitcher->currentIndex(), PasswordBook::ViewType::LIST_VIEW); // 枚举值==位置, 初始化时以固定

      // unchange
      QCOMPARE(pwdBook.onAccountViewChanged(PasswordBook::ViewType::LIST_VIEW), true); // unchange

      // out of range. not crash
      QCOMPARE(pwdBook.onAccountViewChanged(999), false);
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::LIST_VIEW);
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountListView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 0);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::LIST_VIEW], 0);

      // 切TABLE. ok
      QCOMPARE(pwdBook.onAccountViewChanged(PasswordBook::ViewType::TABLE_VIEW), true); // unchange
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::TABLE_VIEW);
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountTableView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 1);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::TABLE_VIEW], 1);
    }

    {
      // 初始为Table
      PasswordBook pwdBook{"", "", PasswordBook::ViewType::TABLE_VIEW, nullptr}; // viewType out of range. not crash
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::TABLE_VIEW);
      QVERIFY(pwdBook.mMainWidget != nullptr);
      QVERIFY(pwdBook.mAccountListView == nullptr);  // lazy launch
      QVERIFY(pwdBook.mAccountTableView != nullptr); // lazy launch
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountTableView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 0);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::TABLE_VIEW], 0);

      QVERIFY(pwdBook.mAccountViewSwitcher != nullptr);
      QCOMPARE(pwdBook.mAccountViewSwitcher->currentIndex(), PasswordBook::ViewType::TABLE_VIEW); // 枚举值==位置, 初始化时以固定

      // unchange
      QCOMPARE(pwdBook.onAccountViewChanged(PasswordBook::ViewType::TABLE_VIEW), true); // unchange

      // out of range. not crash
      QCOMPARE(pwdBook.onAccountViewChanged(999), false);
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::TABLE_VIEW);
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountTableView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 0);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::TABLE_VIEW], 0);

      // 切List. ok
      QCOMPARE(pwdBook.onAccountViewChanged(PasswordBook::ViewType::LIST_VIEW), true); // unchange
      QCOMPARE(pwdBook.mCurViewType, PasswordBook::ViewType::LIST_VIEW);
      QCOMPARE(pwdBook.mMainWidget->currentWidget(), pwdBook.mAccountListView);
      QCOMPARE(pwdBook.mMainWidget->currentIndex(), 1);
      QCOMPARE(pwdBook.mViewType2StackedWidgetIndex[PasswordBook::ViewType::LIST_VIEW], 1);
    }
  }

  void MarkSelectedRowsDeleted_SetFilter_OpenEncFileLocatedIn_ok() {
    const QString locatedInPath{mTDir.path()};
    // 默认 按照Name升序
    PasswordBook pwdBook{mTDir.itemPath(mEncCsvFileName), "REGISTER_AES_KEY", PasswordBook::ViewType::LIST_VIEW, nullptr};
    QVERIFY(pwdBook.mPwdModel != nullptr);
    QVERIFY(pwdBook.mPwdModel->IsDecryptedOk());
    QVERIFY(pwdBook.mSortProxyModel != nullptr);

    QCOMPARE(pwdBook.mPwdModel->rowCount(), 0);
    QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 0);

    QCOMPARE(pwdBook.MarkSelectedRowsDeleted(true), 0); // 没有行->没有行能被选中
    pwdBook.SetFilter("");

    // OpenEncFileLocatedIn
    MOCKER(FileTool::OpenLocalFileUsingDesktopService).expects(atLeast(1)).with(eq(locatedInPath)).will(returnValue(true));
    emit pwdBook.mOpenContainingFolderAct->trigger(); // slots: OpenEncFileLocatedIn

    QList<AccountInfo> accsList
        {
         AccountInfo{"Tool", "Stardew Valley Extension Pack", "Burning Fire", "456", "your life"},
         AccountInfo{"Game", "Minecraft", "DiamondTuber", "123", "Enjoy your life"},
         };
    pwdBook.mPwdModel->AppendAccountRecords(accsList);
    QCOMPARE(pwdBook.mPwdModel->rowCount(), 2);
    QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 2);

    // AppendNRows, InsertNRows
    {
      QVERIFY(pwdBook.mAppendRowAct != nullptr);
      emit pwdBook.mAppendRowAct->trigger();

      QCOMPARE(pwdBook.mPwdModel->rowCount(), 3);
      QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 3);
    }
    /*
      0   AccountInfo{"Tool", "Stardew Valley Extension Pack", "Burning Fire", "456", "your life"},
      1   AccountInfo{"Game", "Minecraft", "DiamondTuber", "123", "Enjoy your life"},
      2   AccountInfo{},
     */

    // SetFilter
    {
      QVERIFY(pwdBook.mSearchText != nullptr);

      pwdBook.mSearchText->setText("your life"); // 0,1
      emit pwdBook.mSearchText->returnPressed();
      QCOMPARE(pwdBook.mPwdModel->rowCount(), 3);
      QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 2);

      pwdBook.mSearchText->setText("Minecraft"); // 2
      emit pwdBook.mSearchText->returnPressed();
      QCOMPARE(pwdBook.mPwdModel->rowCount(), 3);
      QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 1);
      QVERIFY(!pwdBook.ScrollToSourceRow(0)); // no need scroll to
      QVERIFY(pwdBook.ScrollToSourceRow(1));  // need scroll to
      QVERIFY(!pwdBook.ScrollToSourceRow(2)); // no need scroll to

      pwdBook.mSearchText->setText("");          // 3者都匹配
      emit pwdBook.mSearchText->returnPressed();
      QCOMPARE(pwdBook.mPwdModel->rowCount(), 3);
      QCOMPARE(pwdBook.mSortProxyModel->rowCount(), 3);
    }

    // MarkSelectedRowsDeleted
    {
      QVERIFY(pwdBook.mAccountListView != nullptr);
      pwdBook.mAccountListView->selectAll();
      QVERIFY(pwdBook.mAccountListView->selectionModel()->hasSelection());

      QCOMPARE(pwdBook.MarkSelectedRowsDeleted(true), 3);
      QCOMPARE(pwdBook.MarkSelectedRowsDeleted(false), 3);
    }

    // ScrollToSourceRow, 3者都没有被挡住
    {
      QVERIFY(pwdBook.ScrollToSourceRow(0));
      QVERIFY(pwdBook.ScrollToSourceRow(1));
      QVERIFY(pwdBook.ScrollToSourceRow(2));
    }
  }
};

#include "PasswordBookTest.moc"
REGISTER_TEST(PasswordBookTest, false)
