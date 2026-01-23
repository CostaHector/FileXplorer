#ifndef CREDENTIALUTIL_H
#define CREDENTIALUTIL_H

#include <QString>

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
};
#endif

#endif  // CREDENTIALUTIL_H
