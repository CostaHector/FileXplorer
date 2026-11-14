#ifndef PASSWORDBOOKSERVER_H
#define PASSWORDBOOKSERVER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QStringList>
#include <thread>

#include "AccountStorage.h"

namespace httplib {
class Request;
class Response;
}

class PasswordBookServer : public QObject {
  Q_OBJECT
public:
  static constexpr const char* DEFAULT_HOST = "0.0.0.0";
  static constexpr int         DEFAULT_PORT = 8767;

  using QObject::QObject;
  ~PasswordBookServer() override;

  bool setAccounts(const QString& aesKey);

  bool    start(quint16 port = DEFAULT_PORT);
  void    stop();
  bool    isRunning() const { return m_server != nullptr; }
  quint16 port()      const { return m_port; }

  static QStringList lanAddresses();
  QString urlAddress() const { return QString::asprintf("http://127.0.0.1:%d", port()); }

private:
  // 校验 Authorization: Bearer <token>，失败时写 401 并返回 false
  bool CheckToken(const httplib::Request& req, httplib::Response& res) const;

  // GET
  bool onGetAccounts(const httplib::Request& req, httplib::Response& res);
  // POST
  bool onPostSetKey(const httplib::Request& req, httplib::Response& res);
  bool onPostUpdate(const httplib::Request& req, httplib::Response& res);   // 保存某条
  bool onPostDelete(const httplib::Request& req, httplib::Response& res);   // 标记删除
  bool onPostRestore(const httplib::Request& req, httplib::Response& res);  // 恢复删除标记
  bool onPostAdd   (const httplib::Request& req, httplib::Response& res);   // 尾部新增
  bool onPostCommit  (const httplib::Request& req, httplib::Response& res);   // 写入磁盘（占位）

  std::string buildAccountsJson() const;
  static QByteArray indexHtml();

  mutable QMutex m_accountsMutex;
  AccountStorage m_accountStorage;
  mutable QMutex m_tokenMutex;
  QString        m_token;

  quint16     m_port   = 0;
  std::thread m_thread;
  void*       m_server = nullptr;
};

#endif // PASSWORDBOOKSERVER_H
