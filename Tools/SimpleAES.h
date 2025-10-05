#ifndef SIMPLEAES_H
#define SIMPLEAES_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QMessageAuthenticationCode>
#include <QString>
#include <openssl/err.h>
#include <openssl/evp.h>
// #include <openssl/provider.h>
#include <openssl/rand.h>

class SimpleAES {
 private:
  static QString KEY; // 由用户登录时输入
  static bool isValidDecryption(const QString &text) { return !text.trimmed().isEmpty(); }

 public:
  static bool B_USE_RANDOM_IV;
  static void setKey(const QString &userInputKey) {
    if (userInputKey.length() > 16) {
      KEY = userInputKey.left(16);
      return;
    }
    if (userInputKey.length() < 16) {
      int zerosToAdd = 16 - userInputKey.length();
      KEY = userInputKey + QString(zerosToAdd, '0'); // 补全16-length个'0'
      return;
    }
    KEY = userInputKey;
  }

          // ----------------- GCM 模式加密解密 -----------------

  static bool encrypt_GCM(const QString &input, QString& encryptedResult);

  /**
   * @brief 解密 AES-GCM 加密的数据
   * @param input 加密的Base64字符串
   * @param decryptedResult 解密后的明文（输出参数）
   * @return true 解密成功，false 解密失败
   */
  static bool decrypt_GCM(const QString &input, QString &decryptedResult);
};

#endif // SIMPLEAES_H
