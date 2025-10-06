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
/*
sudo apt install libssl-dev needed
openssl version
ls /usr/include/openssl/
 */

class SimpleAES {
 private:
  static constexpr int MAX_KEY_BYTES_LENGTH = 256;
  static unsigned char KEY_BYTES_UNSIGNED_ARRAY[MAX_KEY_BYTES_LENGTH+1];  // 由用户登录时输入
  static bool isValidDecryption(const QString &text) { return !text.trimmed().isEmpty(); }

 public:
  static bool B_USE_RANDOM_IV;
  static void setKey(const QString &userInputKey);

          // ----------------- GCM 模式加密解密 -----------------

  static bool encrypt_GCM(const QString &input, QString& encryptedResult);
  static bool encrypt_GCM_ByteArray(const QByteArray &input, QByteArray& encryptedResult);

  /**
   * @brief 解密 AES-GCM 加密的数据
   * @param input 加密的Base64字符串
   * @param decryptedResult 解密后的明文（输出参数）
   * @return true 解密成功，false 解密失败
   */
  static bool decrypt_GCM(const QString &input, QString &decryptedResult);
  static bool decrypt_GCM_ByteArray(const QByteArray &input, QByteArray &decryptedResult);
};

#endif // SIMPLEAES_H
