#ifndef SIMPLEAES_H
#define SIMPLEAES_H

#include <QByteArray>
#include <QString>

class SimpleAES {
public:
  explicit SimpleAES(const QString& key);
  static SimpleAES &GetInst();
  static void InitInst(const QString& key);

  operator bool() const { return isInited(); }
  bool isInited() const { return m_bInited; }

  bool encrypt_GCM(const QString &input, QString &encryptedResult) const;
  bool encrypt_GCM_ByteArray(const QByteArray &input, QByteArray &encryptedResult) const;

  bool decrypt_GCM(const QString &input, QString &decryptedResult) const;
  bool decrypt_GCM_ByteArray(const QByteArray &input, QByteArray &decryptedResult) const;

private:
  SimpleAES() = default;
  void setKey(const QString &userInputKey);
  void setInited() { m_bInited = true; }
  bool m_bInited{false};
  bool m_bUseRandomIV{true}; // all 0 is used for debug only
  static constexpr int MAX_KEY_BYTES_LENGTH = 256;
  unsigned char mKeyBytesArray[MAX_KEY_BYTES_LENGTH + 1]{0};
};

#endif // SIMPLEAES_H
