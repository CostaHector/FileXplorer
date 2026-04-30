#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "SimpleAES.h"
#include "EndToExposePrivateMember.h"

#ifdef PASSVAULT_ENABLED
#include <openssl/err.h>
#include <openssl/evp.h>
// #include <openssl/provider.h>
#include <openssl/rand.h>
#endif

class SimpleAESTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QString CORRECT_AES_KEY{"MySecretKey12345"};
  SimpleAES mAes{CORRECT_AES_KEY};
private slots:
  void initTestCase() {
#ifdef PASSVAULT_ENABLED
    // 注册 OpenSSL 错误处理
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
#endif
  }

  void cleanupTestCase() {
#ifdef PASSVAULT_ENABLED
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();
#endif
  }

  void init() { mAes.setKey(CORRECT_AES_KEY); }

  void test_encrypt_decrypt_success() {
    QString originalText = "Hello, this is a test message for AES-GCM encryption!";

    QString encryptedText1, decryptedText1;
    {
      QVERIFY(mAes.encrypt_GCM(originalText, encryptedText1));
      QVERIFY(!encryptedText1.isEmpty());
      QVERIFY(mAes.decrypt_GCM(encryptedText1, decryptedText1));
      QCOMPARE(decryptedText1, originalText);
    }

    QString encryptedText2, decryptedText2;
    {
      QVERIFY(mAes.encrypt_GCM(originalText, encryptedText2));
      QVERIFY(!encryptedText2.isEmpty());
      QVERIFY(mAes.decrypt_GCM(encryptedText2, decryptedText2));
      QCOMPARE(decryptedText2, originalText);
    }

#ifdef PASSVAULT_ENABLED
    // openssl with salt
    QVERIFY(encryptedText1 != encryptedText2);
#else
    // no decrypt no salt
    QVERIFY(encryptedText1 == encryptedText2);
#endif
  }

  void decrypt_with_wrong_key() {
    QString originalText = "Another test message with special characters: @#$%^&*()";
    QString encryptedText;
    QString decryptedText;

    // correct key
    QVERIFY(mAes.encrypt_GCM(originalText, encryptedText));

    // wrong key
    mAes.setKey("WrongSecretKey123");

    bool decryptResult = mAes.decrypt_GCM(encryptedText, decryptedText);

#ifdef PASSVAULT_ENABLED
    // openssl decrypt failed, wrong key
    QVERIFY(!decryptResult);
    QVERIFY(decryptedText != originalText);
#else
    // no decrypt at all. succeed ignored wrong key
    QVERIFY(decryptResult);
    QVERIFY(decryptedText == originalText);
#endif
  }

  void empty_string_always_succeed() {
    QString emptyText = "";
    QString encryptedText;
    QString decryptedText;

    QVERIFY(mAes.encrypt_GCM(emptyText, encryptedText));
    QVERIFY(encryptedText.isEmpty());

    QVERIFY(mAes.decrypt_GCM(encryptedText, decryptedText));
    QVERIFY(decryptedText.isEmpty());
  }

  void long_text_succed() {
    QString longText;
    for (int i = 0; i < 10; i++) {
      longText += "This is a long text to test AES-GCM encryption and decryption. ";
    }

    QString encryptedText;
    QString decryptedText;

    QVERIFY(mAes.encrypt_GCM(longText, encryptedText));    
    QVERIFY(mAes.decrypt_GCM(encryptedText, decryptedText));

    QVERIFY(!encryptedText.isEmpty());
    QCOMPARE(decryptedText, longText);
  }

  void special_characters_succeed() {
    QString specialText = "特殊字符测试: 中文, русский, ελληνικά, 日本語, 😊✅🚀";
    QString encryptedText;
    QString decryptedText;

    QVERIFY(mAes.encrypt_GCM(specialText, encryptedText));
    QVERIFY(mAes.decrypt_GCM(encryptedText, decryptedText));
    QCOMPARE(decryptedText, specialText);
  }
};

#include "SimpleAESTest.moc"
REGISTER_TEST(SimpleAESTest, false)
