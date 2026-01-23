#include "CredentialUtil.h"
#include "Logger.h"

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
#define QT_NO_KEYWORDS  // 1. 禁用 Qt 关键字宏
#undef signals
#undef slots
#undef emit
#include <libsecret/secret.h>
#endif

const CredentialUtil& CredentialUtil::GetInst() {
#ifdef _WIN32
  static WinCredUtil winCredUtil;
  return winCredUtil;
#elif defined(__linux__)
  static LinuxCredUtil linuxCredUtil;
  return linuxCredUtil;
#else
  LOG_F("Current platform not support Credential");
  static CredentialUtil defCredUtil;
  return defCredUtil;
#endif
}

bool CredentialUtil::savePassword(const QString& key, const QString& password) const {
  LOG_C("Should never call me");
  return false;
}
QString CredentialUtil::readPassword(const QString& key) const {
  LOG_C("Should never call me");
  return "";
}
bool CredentialUtil::deletePassword(const QString& key) const {
  LOG_C("Should never call me");
  return false;
}
bool CredentialUtil::credentialExists(const QString& key) const {
  LOG_C("Should never call me");
  return false;
}

#ifdef _WIN32
bool WinCredUtil::credentialExists(const QString& key) const {
  PCREDENTIALW pcred;
  return CredReadW(key.toStdWString().c_str(), CRED_TYPE_GENERIC, 0, &pcred);
}

bool WinCredUtil::savePassword(const QString& key, const QString& password) const {
  // 转换字符串为Windows宽字符格式
  std::wstring wkey = key.toStdWString();
  std::wstring wuser = L"User/Aria";  // 可以改为你的应用特定用户标识

  // 准备凭证结构
  CREDENTIALW cred = {0};
  cred.Flags = 0;
  cred.Type = CRED_TYPE_GENERIC;
  cred.TargetName = (LPWSTR)wkey.c_str();
  cred.CredentialBlobSize = password.toUtf8().size();
  cred.CredentialBlob = (LPBYTE)password.toUtf8().data();
  cred.Persist = CRED_PERSIST_LOCAL_MACHINE;  // 持久化到本地机器
  cred.UserName = (LPWSTR)wuser.c_str();

  // 保存凭证
  if (credentialExists(key)) {
    // 如果已存在则先删除
    if (!CredDeleteW(wkey.c_str(), CRED_TYPE_GENERIC, 0)) {
      LOG_W("Failed to delete existing credential");
      return false;
    }
  }

  return CredWriteW(&cred, 0);
}

QString WinCredUtil::readPassword(const QString& key) const {
  PCREDENTIALW pcred;
  std::wstring wkey = key.toStdWString();

  if (CredReadW(wkey.c_str(), CRED_TYPE_GENERIC, 0, &pcred)) {
    QString password = QString::fromUtf8((char*)pcred->CredentialBlob, pcred->CredentialBlobSize);
    CredFree(pcred);
    return password;
  }

  return QString();
}

bool WinCredUtil::deletePassword(const QString& key) const {
  std::wstring wkey = key.toStdWString();
  return CredDeleteW(wkey.c_str(), CRED_TYPE_GENERIC, 0);
}
#endif

#ifdef __linux__
const SecretSchema* getCredentialSchema() {
  static const SecretSchema schema = {
      "com.example.PasswordManager.Credential",
      SECRET_SCHEMA_DONT_MATCH_NAME,
      {
       {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
       {NULL, SECRET_SCHEMA_ATTRIBUTE_STRING},
       }
  };
  return &schema;
}

bool LinuxCredUtil::savePassword(const QString& key, const QString& password) const {
  GError* error = NULL;
  gboolean result = secret_password_store_sync(
      getCredentialSchema(),
      SECRET_COLLECTION_DEFAULT,
      key.toUtf8().constData(), // 凭证名称
      password.toUtf8().constData(),
      NULL, // 加载时回调
      &error,
      "key", key.toUtf8().constData(),
      NULL
      );

  if (error) {
    LOG_E("Failed to save credential: %s", error->message);
    g_error_free(error);
    return false;
  }
  return result;
}

QString LinuxCredUtil::readPassword(const QString& key) const {
  GError* error = NULL;
  gchar* password = secret_password_lookup_sync(
      getCredentialSchema(),
      NULL,
      &error,
      "key", key.toUtf8().constData(),
      NULL
      );

  if (error) {
    LOG_E("Failed to read credential: %s", error->message);
    g_error_free(error);
    return QString();
  }

  if (!password) return "";

  QString result = QString::fromUtf8(password);
  secret_password_free(password);
  return result;
}

bool LinuxCredUtil::deletePassword(const QString& key) const {
  GError* error = NULL;
  gboolean result = secret_password_clear_sync(
      getCredentialSchema(),
      NULL,
      &error,
      "key", key.toUtf8().constData(),
      NULL
      );

  if (error) {
    LOG_E("Failed to delete credential: %s", error->message);
    g_error_free(error);
    return false;
  }
  return result;
}

bool LinuxCredUtil::credentialExists(const QString& key) const {
  GError* error = NULL;
  gchar* password = secret_password_lookup_sync(
      getCredentialSchema(),
      NULL,
      &error,
      "key", key.toUtf8().constData(),
      NULL
      );

  if (error) {
    g_error_free(error);
    return false;
  }

  if (password) {
    secret_password_free(password);
    return true;
  }
  return false;
}
#endif
