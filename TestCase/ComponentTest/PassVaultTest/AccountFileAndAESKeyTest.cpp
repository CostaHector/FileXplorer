#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountFileAndAESKey.h"
#include "EndToExposePrivateMember.h"
#include "CredentialUtil.h"

#include <QSignalSpy>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    class AccountFileAndAESKeyTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void constructor_ok() {
    MOCKER(AccountFileAndAESKey::IsNeedRegisterCore).expects(exactly(2)).will(returnValue(false)).then(returnValue(true));
    // 构造两个对象， 前者预期mAesKeyConfirmLE isHidden为true， 后者为false
    AccountFileAndAESKey noNeedReg{"Path/To/ExistFile.enc.csv", "AES_KEY"};
    QCOMPARE(noNeedReg.mAesKeyConfirmLE->isHidden(), true);

    AccountFileAndAESKey needReg{"Path/To/InexistFile", "AES_KEY"};
    QCOMPARE(needReg.mAesKeyConfirmLE->isHidden(), false);
  }


  void need_register_ok() {
    MOCKER(AccountFileAndAESKey::IsNeedRegisterCore).expects(exactly(3)).will(returnValue(false)).then(returnValue(true)).then(returnValue(false));

    // 构造, 初始情况下 mEncFileFullPathLE 预期 isHidden=true
    // 修改 mEncFileFullPathLE, emit returnPressed, 预期 isHidden=false. emit encFileFullPathChanged
    // 再次修改 mEncFileFullPathLE, emit returnPressed, 预期 isHidden=true, emit encFileFullPathChanged

    AccountFileAndAESKey noNeedReg{"Path/To/ExistFile.enc.csv", "AES_KEY"};
    QCOMPARE(noNeedReg.mAesKeyConfirmLE->isHidden(), true);

    QSignalSpy encFileFullPathChangedSpy{&noNeedReg, &AccountFileAndAESKey::encFileFullPathChanged};
    QVERIFY(encFileFullPathChangedSpy.isValid());

    noNeedReg.mEncFileFullPathLE->setText("Inexist/Path1");
    emit noNeedReg.mEncFileFullPathLE->returnPressed();
    QCOMPARE(noNeedReg.mAesKeyConfirmLE->isHidden(), false);
    QCOMPARE(encFileFullPathChangedSpy.count(), 1);
    QCOMPARE(encFileFullPathChangedSpy.takeLast(), (QVariantList{QString{"Inexist/Path1"}}));

    noNeedReg.mEncFileFullPathLE->setText("Exist/Path2");
    emit noNeedReg.mEncFileFullPathLE->returnPressed();
    QCOMPARE(noNeedReg.mAesKeyConfirmLE->isHidden(), true);
    QCOMPARE(encFileFullPathChangedSpy.count(), 1);
    QCOMPARE(encFileFullPathChangedSpy.takeLast(), (QVariantList{QString{"Exist/Path2"}}));
  }

  void aes_changed_and_apply_when_register_needed_ok() {
    MOCKER(AccountFileAndAESKey::IsNeedRegisterCore).expects(atLeast(1)).will(returnValue(true));

    AccountFileAndAESKey w{"Path/To/InexistFile", "AES_KEY"};
    QCOMPARE(w.mAesKeyConfirmLE->isHidden(), false);   // 需要注册 → Confirm Key 可见

    QSignalSpy aesKeyChangedSpy{&w, &AccountFileAndAESKey::aesKeyChanged};
    QSignalSpy applySpy{&w, &AccountFileAndAESKey::encFileFullPathAndAesKeyChanged};
    QVERIFY(aesKeyChangedSpy.isValid());
    QVERIFY(applySpy.isValid());

    // 1) 两次 key 不一致 → returnPressed 不 emit
    w.mAesKeyLE->setText("newKey1");
    w.mAesKeyConfirmLE->setText("newKey2");
    emit w.mAesKeyLE->returnPressed();
    QCOMPARE(aesKeyChangedSpy.count(), 0);

    // 2) 两次 key 一致 → returnPressed emit aesKeyChanged
    w.mAesKeyConfirmLE->setText("newKey1");
    emit w.mAesKeyLE->returnPressed();
    QCOMPARE(aesKeyChangedSpy.count(), 1);
    QCOMPARE(aesKeyChangedSpy.takeLast(), (QVariantList{QString{"newKey1"}}));

    // 3) 一致时点击 Apply → emit encFileFullPathAndAesKeyChanged
    w.mApplyButton->click();
    QCOMPARE(applySpy.count(), 1);
    QCOMPARE(applySpy.takeLast(),
             (QVariantList{QString{"Path/To/InexistFile"}, QString{"newKey1"}}));

    // 4) 改成不一致 → Apply 不 emit
    w.mAesKeyConfirmLE->setText("diffKey");
    w.mApplyButton->click();
    QCOMPARE(applySpy.count(), 0);
  }

  void aes_changed_and_apply_when_register_not_needed_ok() {
    MOCKER(AccountFileAndAESKey::IsNeedRegisterCore).expects(atLeast(1)).will(returnValue(false));

    AccountFileAndAESKey w{"Path/To/ExistFile.enc.csv", "AES_KEY"};
    QCOMPARE(w.mAesKeyConfirmLE->isHidden(), true);    // 不需要注册 → Confirm Key 隐藏

    QSignalSpy aesKeyChangedSpy{&w, &AccountFileAndAESKey::aesKeyChanged};
    QSignalSpy applySpy{&w, &AccountFileAndAESKey::encFileFullPathAndAesKeyChanged};
    QVERIFY(aesKeyChangedSpy.isValid());
    QVERIFY(applySpy.isValid());

    // 不需要注册时，两次 key 不一致也放行（ValidateAesKeyPair 直接 return true）
    w.mAesKeyLE->setText("newKey1");
    w.mAesKeyConfirmLE->setText("diffKey");
    emit w.mAesKeyLE->returnPressed();
    QCOMPARE(aesKeyChangedSpy.count(), 1);
    QCOMPARE(aesKeyChangedSpy.takeLast(), (QVariantList{QString{"newKey1"}}));

    w.mApplyButton->click();
    QCOMPARE(applySpy.count(), 1);
    QCOMPARE(applySpy.takeLast(), (QVariantList{QString{"Path/To/ExistFile.enc.csv"}, QString{"newKey1"}}));
  }

  void onSaveCurrentInputKey_ok() {
    MOCKER(AccountFileAndAESKey::IsNeedRegisterCore).expects(atLeast(1)).will(returnValue(false));

    // 构造函数里 InitAesKey("AES_KEY") 把两个 LE 都填成 "AES_KEY"，
    // 点击 Save Key 时 GetAesKey() 返回的就是它
    MOCKER(CredUtilHelper::savePassword)
        .expects(exactly(2))
        .with(eq(QString("PASSWORD_MANAGER_AES_KEY_TEST")), eq(QString{"AES_KEY"}))
        .will(returnValue(true));

    AccountFileAndAESKey w{"Path/To/ExistFile.enc.csv", "AES_KEY"};
    w.mSaveCurrentAesKeyButton->click();
    QVERIFY(w.onSaveCurrentInputKey());
    // expects(exactly(1)) 由 cleanup() 里的 GlobalMockObject::verify() 校验
  }
};

#include "AccountFileAndAESKeyTest.moc"
REGISTER_TEST(AccountFileAndAESKeyTest, false)