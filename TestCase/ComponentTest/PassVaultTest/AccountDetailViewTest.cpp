#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountDetailView.h"
#include "EndToExposePrivateMember.h"

class AccountDetailViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_sequential_operations() {
    // 步骤1: 创建视图并验证初始状态
    AccountDetailView view("Account Detail");
    QVERIFY(view.etType != nullptr);
    QVERIFY(view.etName != nullptr);
    QVERIFY(view.etAccount != nullptr);
    QVERIFY(view.etPwd != nullptr);
    QVERIFY(view.etOthers != nullptr);
    QVERIFY(view.mBtnRecover != nullptr);
    QVERIFY(view.mBtnApply != nullptr);

    // 初始状态验证
    QVERIFY(view.etType->text().isEmpty());
    QVERIFY(view.etName->text().isEmpty());
    QVERIFY(view.etAccount->text().isEmpty());
    QVERIFY(view.etPwd->text().isEmpty());
    QVERIFY(view.etOthers->toPlainText().isEmpty());
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());

    // 步骤2: 更新显示账户信息, 禁用Apply,Recover按钮
    AccountInfo account{"Email", "Personal", "user@example.com", "pass123", "Additional info"};
    view.UpdateDisplay(&account);
    QCOMPARE(view.etType->text(), "Email");
    QCOMPARE(view.etName->text(), "Personal");
    QCOMPARE(view.etAccount->text(), "user@example.com");
    QCOMPARE(view.etPwd->text(), "pass123");
    QCOMPARE(view.etOthers->toPlainText(), "Additional info");
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());

    // 步骤3: 修改账户信息, 使能Apply,Recover按钮
    view.etType->setText("Work Email");
    view.etName->setText("Professional");
    view.etAccount->setText("work@example.com");
    view.etPwd->setText("strongerPass");
    view.etOthers->setPlainText("Work account");
    QVERIFY(view.mBtnRecover->isEnabled());
    QVERIFY(view.mBtnApply->isEnabled());

    // 步骤4: 点击恢复按钮, 验证恢复后状态=传参account值, 禁用Apply,Recover按钮
    view.onRecoverModify();
    QCOMPARE(view.etType->text(), "Email");
    QCOMPARE(view.etName->text(), "Personal");
    QCOMPARE(view.etAccount->text(), "user@example.com");
    QCOMPARE(view.etPwd->text(), "pass123");
    QCOMPARE(view.etOthers->toPlainText(), "Additional info");
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());

    // 步骤5: 再次修改账户信息, 使能Apply,Recover按钮
    view.etType->setText("Work Email");
    view.etName->setText("Professional");
    view.etAccount->setText("work@example.com");
    view.etPwd->setText("strongerPass");
    view.etOthers->setPlainText("Work account");
    QVERIFY(view.mBtnRecover->isEnabled());
    QVERIFY(view.mBtnApply->isEnabled());

    // 步骤6: 点击应用按钮, 入参修改同面板显示一致, 禁用Apply,Recover按钮
    view.onApplyModify();
    QCOMPARE(account, (AccountInfo{"Work Email", "Professional", "work@example.com", "strongerPass", "Work account"}));
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());

    // 步骤7: 清空显示, Apply,Recover禁用
    view.UpdateDisplay(nullptr);
    QVERIFY(view.etType->text().isEmpty());
    QVERIFY(view.etName->text().isEmpty());
    QVERIFY(view.etAccount->text().isEmpty());
    QVERIFY(view.etPwd->text().isEmpty());
    QVERIFY(view.etOthers->toPlainText().isEmpty());
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());
    // 步骤8: 尝试修改空视图, Apply,Recover禁用
    view.etType->setText("Test");
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());

    // 步骤9: 点击恢复按钮, 无法恢复
    view.onRecoverModify();
    QVERIFY(!view.mBtnRecover->isEnabled());
    QVERIFY(!view.mBtnApply->isEnabled());
  }
};

#include "AccountDetailViewTest.moc"
REGISTER_TEST(AccountDetailViewTest, false)
