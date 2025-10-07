#ifndef CREDENTIALUTIL_H
#define CREDENTIALUTIL_H

#include <QString>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <wincred.h>
// clang-format on
#elif defined(__linux__)
/*
 sudo apt install libsecret-1-dev
 ls /usr/include/libsecret-1/libsecret/
 */
#define signals signalsVariable
#include <libsecret/secret.h>
#undef signals
#endif

class CredentialUtil {
 public:
  static const CredentialUtil& GetInst();
  virtual ~CredentialUtil() = default;
  virtual bool savePassword(const QString& key, const QString& password) const;
  virtual QString readPassword(const QString& key) const;
  virtual bool deletePassword(const QString& key) const;

 protected:
  virtual bool credentialExists(const QString& key) const;
};

#ifdef _WIN32
class WinCredUtil : public CredentialUtil {
 public:
  bool savePassword(const QString& key, const QString& password) const override;
  QString readPassword(const QString& key) const override;
  bool deletePassword(const QString& key) const override;

 protected:
  bool credentialExists(const QString& key) const override;
};
#elif defined(__linux__)
class LinuxCredUtil : public CredentialUtil {
 public:
  bool savePassword(const QString& key, const QString& password) const override;
  QString readPassword(const QString& key) const override;
  bool deletePassword(const QString& key) const override;

 protected:
  bool credentialExists(const QString& key) const override;
 private:
  const SecretSchema* getCredentialSchema() const;
};
#endif

#endif  // CREDENTIALUTIL_H
