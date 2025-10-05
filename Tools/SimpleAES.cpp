#include "SimpleAES.h"

QString SimpleAES::KEY = "";
bool SimpleAES::B_USE_RANDOM_IV = true;  // all 0 is only for debug

bool SimpleAES::encrypt_GCM(const QString& input, QString& encryptedResult) {
  encryptedResult.clear();
  if (input.isEmpty()) {
    return true;  // 空串无需解密
  }

  // 1. 获取随机 IV
  unsigned char iv[12]{0};
  if (B_USE_RANDOM_IV) {
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
      qWarning("Failed to generate IV");
      return false;
    }
  }

  // 2. 初始化加密器
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, reinterpret_cast<const unsigned char*>(KEY.toUtf8().constData()), iv);

  // 3. 加密数据
  QByteArray plaintext = input.toUtf8();
  int ciphertext_len = plaintext.size() + EVP_CIPHER_block_size(EVP_aes_128_gcm());
  unsigned char* ciphertext = new unsigned char[ciphertext_len];

  int len, total_len = 0;
  EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<const unsigned char*>(plaintext.constData()), plaintext.size());
  total_len = len;

  // 4. 最终加密（生成标签）
  EVP_EncryptFinal_ex(ctx, ciphertext + total_len, &len);
  total_len += len;

  // 5. 获取16字节标签
  unsigned char tag[16];
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);

  // 6. 组合 IV + 密文 + 标签
  QByteArray combined;
  combined.append(reinterpret_cast<char*>(iv), 12);
  combined.append(reinterpret_cast<char*>(ciphertext), total_len);
  combined.append(reinterpret_cast<char*>(tag), 16);

  // 7. Base64编码（关键修改：保留填充符）
  encryptedResult = combined.toBase64(QByteArray::KeepTrailingEquals);  // 保留末尾填充符号

  delete[] ciphertext;
  EVP_CIPHER_CTX_free(ctx);
  return true;
}

bool SimpleAES::decrypt_GCM(const QString& input, QString& decryptedResult) {
  decryptedResult.clear();
  if (input.isEmpty()) {
    return true;  // 无需解密
  }

  // 1. Base64 解码
  QByteArray combined = QByteArray::fromBase64(input.toUtf8());
  if (combined.size() < 12 + 16) {  // 至少包含 IV (12) + 标签 (16)
    qWarning("Ciphertext too short: %d bytes (min 28 required)", combined.size());
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
    qWarning("Failed to create cipher context");
    return false;
  }

  QByteArray keyBytes = KEY.toUtf8();
  const unsigned char* key = reinterpret_cast<const unsigned char*>(keyBytes.constData());

  if (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, key, iv) != 1) {
    qWarning("Failed to initialize decryption");
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
    qWarning("Failed to decrypt data");
    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  total_len = len;

  // 5.2 验证标签（如果失败会返回0）
  if (EVP_DecryptFinal_ex(ctx, plaintext + total_len, &len) != 1) {
    qWarning("GCM tag verification failed");
    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  total_len += len;

  // 6. 转换为QString
  decryptedResult = QString::fromUtf8(reinterpret_cast<const char*>(plaintext), total_len);

  // 7. 清理资源
  delete[] plaintext;
  EVP_CIPHER_CTX_free(ctx);

  // 8. 验证结果
  if (decryptedResult.isEmpty()) {
    qWarning("Decrypted content is empty");
    return false;
  }

  return true;
}
