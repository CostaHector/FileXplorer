#ifndef CREDENTIALUTIL_H
#define CREDENTIALUTIL_H

#include <QString>

class CredentialUtil {
 public:
  friend class CredUtilHelper;
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
  friend class CredUtilHelper;
  bool savePassword(const QString& key, const QString& password) const override;
  QString readPassword(const QString& key) const override;
  bool deletePassword(const QString& key) const override;

 protected:
  bool credentialExists(const QString& key) const override;
};
#elif defined(__linux__)
class LinuxCredUtil : public CredentialUtil {
 public:
  friend class CredUtilHelper;
  bool savePassword(const QString& key, const QString& password) const override;
  QString readPassword(const QString& key) const override;
  bool deletePassword(const QString& key) const override;

 protected:
  bool credentialExists(const QString& key) const override;
};
#endif

struct CredUtilHelper {
  static bool savePassword(const QString& key, const QString& password);
  static QString readPassword(const QString& key);
  static bool deletePassword(const QString& key);
  static bool credentialExists(const QString& key);
};

#endif  // CREDENTIALUTIL_H
