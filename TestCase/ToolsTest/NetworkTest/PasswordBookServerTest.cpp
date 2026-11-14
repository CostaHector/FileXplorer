#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "PasswordBookServer.h"
#include "EndToExposePrivateMember.h"

#include "httplib.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS


class PasswordBookServerTest : public PlainTestSuite {
  Q_OBJECT
public:

private slots:
  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void CheckToken_ok() {
    PasswordBookServer server;
    server.m_token = "valid_token";

    auto makeReq = [](const char* auth) {
      httplib::Request req;
      if (auth) req.set_header("Authorization", auth);
      return req;
    };

    {
      httplib::Response res;
      QVERIFY(!server.CheckToken(makeReq(nullptr), res));
      QCOMPARE(res.status, 401);
    }
    {
      httplib::Response res;
      QVERIFY(!server.CheckToken(makeReq("Invalid"), res));
      QCOMPARE(res.status, 401);
    }
    {
      httplib::Response res;
      QVERIFY(!server.CheckToken(makeReq("Bearer wrong"), res));
      QCOMPARE(res.status, 401);
    }
    {
      httplib::Response res;
      QVERIFY(server.CheckToken(makeReq("Bearer valid_token"), res));
    }
  }

  void setAccounts_wrong_key_after_unlock_returns_false() {
    MOCKER(AccountFileHelper::GetDefaultEncFileFullPath).stubs().will(returnValue(QString{"fake/path"}));

    QList<AccountInfo> accList1{
      AccountInfo{"Game", "Minecraft", "DiamondTuber", "123", "..."},
    };
    QList<AccountInfo> accList2{
      AccountInfo{"Game", "Minecraft", "DiamondTuber", "123", "Duck"},
      AccountInfo{"Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
    };

    MOCKER(AccountFileHelper::ValidateAndLoad)
        .expects(exactly(3))
        .will(returnValue(AccountFileHelper::LoadResult{true,  accList1}))
        .then(returnValue(AccountFileHelper::LoadResult{false, {}}))
        .then(returnValue(AccountFileHelper::LoadResult{true,  accList2}));

    PasswordBookServer server;

    // 首次： 成功解锁才能swap数据, 后续只返回解码成功与否, 不进行swap, 避免多人操作时一人输错, 数据清空了
    QVERIFY(server.setAccounts("correct_key"));
    QVERIFY(server.m_token.isEmpty());   // setAccounts 本身不生成 token
    QCOMPARE(server.m_accountStorage.IsValid(), true);
    QCOMPARE(server.m_accountStorage.mAccounts, accList1);

    // 模拟已解锁（onPostSetKey 才会生成 token）
    server.m_token = "existing_token";

    // 已解锁后：错误 key 必须返回 false（关键回归）
    QVERIFY(!server.setAccounts("wrong_key"));
    QCOMPARE(server.m_accountStorage.IsValid(), true);
    QCOMPARE(server.m_accountStorage.mAccounts, accList1);

    // 已解锁后：正确 key 仍然返回 true, 解锁成功
    QVERIFY(server.setAccounts("correct_key"));
    QCOMPARE(server.m_accountStorage.IsValid(), true);
    QCOMPARE(server.m_accountStorage.mAccounts, accList1);
  }

  void onPostSetKey_first_time_generates_token_and_reuses_it() {
    MOCKER(AccountFileHelper::GetDefaultEncFileFullPath).stubs().will(returnValue(QString{"fake/path"}));
    MOCKER(AccountFileHelper::ValidateAndLoad)
        .expects(exactly(3))
        .then(returnValue(AccountFileHelper::LoadResult{true, {}}))
        .then(returnValue(AccountFileHelper::LoadResult{true, {}}))
        .then(returnValue(AccountFileHelper::LoadResult{false, {}}));

    PasswordBookServer server;

    auto post = [&](const QString& key) {
      httplib::Request req;
      req.body = QJsonDocument(QJsonObject{{"key", key}}).toJson(QJsonDocument::Compact).toStdString();
      httplib::Response res;
      server.onPostSetKey(req, res);
      return std::make_pair(res, QJsonDocument::fromJson(QByteArray::fromStdString(res.body)).object());
    };

    httplib::Response res1, res2, res3;
    QJsonObject obj1, obj2, obj3;

    // 首次：生成 token
    std::tie(res1, obj1) = post("correct_key");
    QCOMPARE(res1.status, 200);
    QVERIFY(obj1["ok"].toBool());
    const QString token1 = obj1["token"].toString();
    QVERIFY(!token1.isEmpty());

    // 再次：返回同一个 token
    std::tie(res2, obj2) = post("correct_key");
    QCOMPARE(res2.status, 200);
    QCOMPARE(obj2["token"].toString(), token1);

    // 错误 key：401，token 不变
    std::tie(res3, obj3) = post("wrong_key");
    QCOMPARE(res3.status, 401);
    QCOMPARE(server.m_token, token1);   // 全局 token 没被清
  }


  void all_handlers_reject_without_token_data() {
    QTest::addColumn<QString>("handler");
    QTest::newRow("update")  << "update";
    QTest::newRow("delete")  << "delete";
    QTest::newRow("restore") << "restore";
    QTest::newRow("add")     << "add";
    QTest::newRow("commit")  << "commit";
    QTest::newRow("accounts")<< "accounts";
  }

  void all_handlers_reject_without_token() {
    QFETCH(QString, handler);
    PasswordBookServer server;
    server.m_token = "valid_token";

    httplib::Request req;
    httplib::Response res;

    if (handler == "accounts")      server.onGetAccounts(req, res);
    else if (handler == "update")   server.onPostUpdate(req, res);
    else if (handler == "delete")   server.onPostDelete(req, res);
    else if (handler == "restore")  server.onPostRestore(req, res);
    else if (handler == "add")      server.onPostAdd(req, res);
    else if (handler == "commit")   server.onPostCommit(req, res);

    QCOMPARE(res.status, 401);
  }
};

#include "PasswordBookServerTest.moc"
REGISTER_TEST(PasswordBookServerTest, false)