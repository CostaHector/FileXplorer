#include "SimpleAES.h"
#include "Logger.h"
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>

#include <openssl/err.h>
#include <openssl/evp.h>
// #include <openssl/provider.h>
#include <openssl/rand.h>
/*
sudo apt install libssl-dev needed
openssl version
ls /usr/include/openssl/
 */

constexpr int SimpleAES::MAX_KEY_BYTES_LENGTH;

SimpleAES &SimpleAES::GetInst() {
  static SimpleAES inst;
  return inst;
}

void SimpleAES::InitInst(const QString& key) {
  GetInst().setKey(key);
}

SimpleAES::SimpleAES(const QString& key) {
  setKey(key);
}

void SimpleAES::setKey(const QString& userInputKey) {
  if (isInited()) {
    LOG_W("password already set. now key changed");
  }
  QString keyString;
  if (userInputKey.length() > 16) {
    keyString = userInputKey.left(16);
  } else if (userInputKey.length() < 16) {
    int zerosToAdd = 16 - userInputKey.length();
    keyString = userInputKey + QString(zerosToAdd, '0');  // append 16-length numbers of char '0'
  } else {
    keyString = userInputKey;
  }

  QByteArray keyBytes = keyString.toUtf8();
  const int validLen = static_cast<size_t>(std::min<long long>(keyBytes.size(), MAX_KEY_BYTES_LENGTH));
  memcpy(mKeyBytesArray, keyBytes.constData(), validLen);
  mKeyBytesArray[validLen] = '\0';

  setInited();
}

bool SimpleAES::encrypt_GCM_ByteArray(const QByteArray& input, QByteArray& encryptedResult) const {
  if (!isInited()) {
    LOG_E("encrypted key not Inited");
    return false;
  }
  encryptedResult.clear();
  if (input.isEmpty()) {
    return true;  // 空串无需解密
  }

  // 1. 获取随机 IV
  unsigned char iv[12]{0};
  if (m_bUseRandomIV) {
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
      LOG_W("Failed to generate IV");
      return false;
    }
  }

  // 2. 初始化加密器
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, mKeyBytesArray, iv);

  // 3. 加密数据
  int ciphertext_len = input.size() + EVP_CIPHER_block_size(EVP_aes_128_gcm());
  unsigned char* ciphertext = new unsigned char[ciphertext_len];

  int len, total_len = 0;
  EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<const unsigned char*>(input.constData()), input.size());
  total_len = len;

  // 4. 最终加密（生成标签）
  EVP_EncryptFinal_ex(ctx, ciphertext + total_len, &len);
  total_len += len;

  // 5. 获取16字节标签
  unsigned char tag[16];
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);

  // 6. 组合 IV + 密文 + 标签
  encryptedResult.append(reinterpret_cast<char*>(iv), 12);
  encryptedResult.append(reinterpret_cast<char*>(ciphertext), total_len);
  encryptedResult.append(reinterpret_cast<char*>(tag), 16);

  delete[] ciphertext;
  EVP_CIPHER_CTX_free(ctx);
  return true;
}

bool SimpleAES::encrypt_GCM(const QString& input, QString& encryptedResult) const {
  if (!isInited()) {
    LOG_E("encrypted key not Inited");
    return false;
  }
  QByteArray inputByteArray = input.toUtf8();
  QByteArray outputByteArray;
  if (!encrypt_GCM_ByteArray(inputByteArray, outputByteArray)) {
    return false;
  }
  if (!outputByteArray.isEmpty()) {
    encryptedResult = outputByteArray.toBase64(QByteArray::KeepTrailingEquals);  // 保留末尾填充符号
  }
  return true;
}

bool SimpleAES::decrypt_GCM_ByteArray(const QByteArray& input, QByteArray& decryptedResult) const {
  if (!isInited()) {
    LOG_E("encrypted key not Inited");
    return false;
  }

  decryptedResult.clear();
  if (input.isEmpty()) {
    return true;  // 无需解密
  }

  // 1. Base64 解码
  QByteArray combined = QByteArray::fromBase64(input);
  if (combined.size() < 12 + 16) {  // 至少包含 IV (12) + 标签 (16)
    LOG_W("Ciphertext too short: %d bytes (min 28 required)", combined.size());
    return false;
  }

  // 2. 分离 IV (12字节)、密文和标签 (最后16字节)
  unsigned char iv[12];
  memcpy(iv, combined.constData(), sizeof(iv));

  QByteArray ciphertextWithTag = combined.mid(12);
  QByteArray ciphertext = ciphertextWithTag.left(ciphertextWithTag.size() - 16);  // 移除最后16字节标签
  unsigned char tag[16];
  memcpy(tag, ciphertextWithTag.constData() + ciphertext.size(), sizeof(tag));

  // 3. 初始化解密器
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    LOG_W("Failed to create cipher context");
    return false;
  }

  if (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, mKeyBytesArray, iv) != 1) {
    LOG_W("Failed to initialize decryption");
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  // 4. 设置预期的GCM标签（关键步骤！）
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);

  // 5. 解密数据
  int len, total_len = 0;
  int plaintext_len = ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_128_gcm());
  unsigned char* plaintext = new unsigned char[plaintext_len];

  // 5.1 解密主体数据
  if (EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<const unsigned char*>(ciphertext.constData()), ciphertext.size()) != 1) {
    LOG_W("Failed to decrypt data");
    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  total_len = len;

  // 5.2 验证标签（如果失败会返回0）
  if (EVP_DecryptFinal_ex(ctx, plaintext + total_len, &len) != 1) {
    LOG_W("GCM tag verification failed");
    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  total_len += len;

  // 6. 转换为QString
  QByteArray tmpBA(reinterpret_cast<const char*>(plaintext), total_len);
  decryptedResult.swap(tmpBA);

  // 7. 清理资源
  delete[] plaintext;
  EVP_CIPHER_CTX_free(ctx);

  // 8. 验证结果
  if (decryptedResult.isEmpty()) {
    LOG_W("Decrypted content is empty");
    return false;
  }

  return true;
}

bool SimpleAES::decrypt_GCM(const QString& input, QString& decryptedResult) const {
  if (!isInited()) {
    LOG_E("encrypted key not Inited");
    return false;
  }
  QByteArray inputBa = input.toUtf8();
  QByteArray outputBa;
  decryptedResult.clear();
  if (!decrypt_GCM_ByteArray(inputBa, outputBa)) {
    return false;
  }
  if (!outputBa.isEmpty()) {
    decryptedResult = QString::fromUtf8(outputBa);
  }
  return true;
}
