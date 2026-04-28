#include <QtTest/QtTest>
#include "PlainTestSuite.h"


#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "CredentialUtil.h"
#include "EndToExposePrivateMember.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CredentialUtilTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  QString mCredKeyName{"CredentialUtilKey"};
  QString mSystemDependentCredKeyName{"SystemDependentCredentialUtilKey"};
 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    QVERIFY(mTDir.IsValid());
    MOCKER(QStandardPaths::writableLocation).stubs().will(returnValue(mTDir.path()));
  }

  void cleanupTestCase() {  //
    GlobalMockObject::verify();

    const CredentialUtil& credUtil = CredentialUtil::GetInst();
    if (credUtil.credentialExists(mSystemDependentCredKeyName)) {
      credUtil.deletePassword(mSystemDependentCredKeyName);
    }
  }

  void CredentialUtil_ok() {  //
    const CredentialUtil credUtil;
    QVERIFY(!mTDir.exists("pwdEncFile.txt"));
    QVERIFY(!credUtil.credentialExists(mCredKeyName));
    QVERIFY(credUtil.savePassword(mCredKeyName, "PasswordHello"));
    QVERIFY(mTDir.exists("pwdEncFile.txt"));

    QCOMPARE(credUtil.readPassword(mCredKeyName), "PasswordHello");
    QVERIFY(credUtil.credentialExists(mCredKeyName));
    QVERIFY(credUtil.deletePassword(mCredKeyName));
    QVERIFY(!credUtil.credentialExists(mCredKeyName));
  }

  void platformBasedUtil_ok() {
    const CredentialUtil& credUtil = CredentialUtil::GetInst();
    QVERIFY(!credUtil.credentialExists(mSystemDependentCredKeyName));
    QVERIFY(credUtil.savePassword(mSystemDependentCredKeyName, "PasswordHello"));
    QCOMPARE(credUtil.readPassword(mSystemDependentCredKeyName), "PasswordHello");
    QVERIFY(credUtil.credentialExists(mSystemDependentCredKeyName));
    QVERIFY(credUtil.deletePassword(mSystemDependentCredKeyName));
    QVERIFY(!credUtil.credentialExists(mSystemDependentCredKeyName));
  }
};

#include "CredentialUtilTest.moc"
REGISTER_TEST(CredentialUtilTest, false)
