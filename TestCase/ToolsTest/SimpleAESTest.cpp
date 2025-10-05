#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SimpleAES.h"
#include "EndToExposePrivateMember.h"

#include "OnScopeExit.h"

class SimpleAESTest : public PlainTestSuite {
  Q_OBJECT
 public:
  const QString CORRECT_AES_KEY{"MySecretKey12345"};
 private slots:
  void initTestCase() {
    // 注册 OpenSSL 错误处理
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    // 初始化密钥
    SimpleAES::setKey(CORRECT_AES_KEY);  // 16字符密钥
  }

  void cleanupTestCase() {
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();
  }

  void test_encrypt_decrypt_success() {
    QString originalText = "Hello, this is a test message for AES-GCM encryption!";
    QString encryptedText;
    QString decryptedText;

    QVERIFY(SimpleAES::encrypt_GCM(originalText, encryptedText));
    QVERIFY(!encryptedText.isEmpty());

    QVERIFY(SimpleAES::decrypt_GCM(encryptedText, decryptedText));

    QCOMPARE(decryptedText, originalText);
  }

  void test_decrypt_with_wrong_key() {
    QString originalText = "Another test message with special characters: @#$%^&*()";
    QString encryptedText;
    QString decryptedText;

    // 使用正确密钥加密
    QVERIFY(SimpleAES::encrypt_GCM(originalText, encryptedText));

    // 修改密钥为错误值
    SimpleAES::setKey("WrongSecretKey123");
    ON_SCOPE_EXIT {
      SimpleAES::setKey(CORRECT_AES_KEY);
    };

    // 尝试解密（应失败）
    bool decryptResult = SimpleAES::decrypt_GCM(encryptedText, decryptedText);

    // 验证解密失败
    QVERIFY(!decryptResult);
    QVERIFY(decryptedText != originalText);
  }

  void test_empty_string() {
    QString emptyText = "";
    QString encryptedText;
    QString decryptedText;

    QVERIFY(SimpleAES::encrypt_GCM(emptyText, encryptedText));
    QVERIFY(encryptedText.isEmpty());

    QVERIFY(SimpleAES::decrypt_GCM(encryptedText, decryptedText));
    QVERIFY(decryptedText.isEmpty());
  }

  void test_long_text() {
    QString longText;
    for (int i = 0; i < 10; i++) {
      longText += "This is a long text to test AES-GCM encryption and decryption. ";
    }

    QString encryptedText;
    QString decryptedText;

    QVERIFY(SimpleAES::encrypt_GCM(longText, encryptedText));
    QVERIFY(!encryptedText.isEmpty());

    QVERIFY(SimpleAES::decrypt_GCM(encryptedText, decryptedText));

    QCOMPARE(decryptedText, longText);
  }

  void test_special_characters() {
    QString specialText = "特殊字符测试: 中文, русский, ελληνικά, 日本語, 😊✅🚀";
    QString encryptedText;
    QString decryptedText;

    QVERIFY(SimpleAES::encrypt_GCM(specialText, encryptedText));
    QVERIFY(SimpleAES::decrypt_GCM(encryptedText, decryptedText));
    QCOMPARE(decryptedText, specialText);
  }
};

#include "SimpleAESTest.moc"
REGISTER_TEST(SimpleAESTest, false)
