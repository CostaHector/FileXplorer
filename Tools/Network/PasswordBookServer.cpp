#include "PasswordBookServer.h"

#include "httplib.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QFile>

PasswordBookServer::~PasswordBookServer() { stop(); }

bool PasswordBookServer::setAccounts(const QString& aesKey) {
  QMutexLocker lock(&m_accountsMutex);

  QString encFilePath = AccountFileHelper::GetDefaultEncFileFullPath();
  AccountStorage temp{encFilePath, aesKey};

  {
    QMutexLocker tokenLock(&m_tokenMutex);
    if (!m_token.isEmpty()) {
      // 已解密： 只验证这次 key 是否有效，不 swap（保持已有数据和修改）
      return temp.IsValid();
    }
  }

  m_accountStorage.swap(temp);
  return m_accountStorage.IsValid();
}

// ---------- 启动 / 停止 ----------

bool PasswordBookServer::start(quint16 port) {
  if (m_server != nullptr) return true;

  httplib::Server* svr = new (std::nothrow) httplib::Server();
  if (!svr) return false;
  m_server = svr;

  if (!svr->bind_to_port(DEFAULT_HOST, port)) {
    qWarning("PasswordServer: cannot bind %s:%d (port busy?)", DEFAULT_HOST, port);
    stop();
    return false;
  }
  m_port = port;

  svr->Get("/", [](const httplib::Request&, httplib::Response& res) {
    const QByteArray html = indexHtml();
    res.set_content(std::string(html.constData(), size_t(html.size())),
                    "text/html; charset=utf-8");
    res.set_header("Cache-Control", "no-cache");
  });

  svr->Get("/api/accounts", [this](const httplib::Request& req, httplib::Response& res) {
    onGetAccounts(req, res);
  });

  svr->Post("/api/setkey", [this](const httplib::Request& req, httplib::Response& res) {
    onPostSetKey(req, res);
  });

  svr->Post("/api/update", [this](const httplib::Request& req, httplib::Response& res) {
    onPostUpdate(req, res);
  });

  svr->Post("/api/delete", [this](const httplib::Request& req, httplib::Response& res) {
    onPostDelete(req, res);
  });

  svr->Post("/api/restore", [this](const httplib::Request& req, httplib::Response& res) {
    onPostRestore(req, res);
  });

  svr->Post("/api/add", [this](const httplib::Request& req, httplib::Response& res) {
    onPostAdd(req, res);
  });

  svr->Post("/api/commit", [this](const httplib::Request& req, httplib::Response& res) {
    onPostCommit(req, res);
  });

  m_thread = std::thread([svr]() { svr->listen_after_bind(); });
  return true;
}

void PasswordBookServer::stop() {
  if (m_server) static_cast<httplib::Server*>(m_server)->stop();
  if (m_thread.joinable()) m_thread.join();
  delete static_cast<httplib::Server*>(m_server);
  m_server = nullptr;
  m_port = 0;
}

bool PasswordBookServer::CheckToken(const httplib::Request& req, httplib::Response& res) const {
  static const std::string kPrefix = "Bearer ";
  const std::string auth = req.get_header_value("Authorization");
  if (auth.size() <= kPrefix.size() || auth.compare(0, kPrefix.size(), kPrefix) != 0) {
    res.status = 401;
    res.set_content(R"({"ok":false,"error":"missing or malformed token"})", "application/json");
    return false;
  }
  const QString token = QString::fromStdString(auth.substr(kPrefix.size()));
  QMutexLocker lock(&m_tokenMutex);
  if (m_token != token) {
    res.status = 401;
    res.set_content(R"({"ok":false,"error":"invalid or expired token"})", "application/json");
    return false;
  }
  return true;
}

// ---------- GET /api/accounts ----------

bool PasswordBookServer::onGetAccounts(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;

  res.set_content(buildAccountsJson(), "application/json; charset=utf-8");
  res.set_header("Cache-Control", "no-store, no-cache, must-revalidate");
  res.set_header("Pragma", "no-cache");
  if (res.status == -1) res.status = 200;
  return true;
}

std::string PasswordBookServer::buildAccountsJson() const {
  QMutexLocker lock(&m_accountsMutex);

  QJsonArray arr;
  for (int i = 0; i < m_accountStorage.size(); ++i) {
    const AccountInfo& a = m_accountStorage[i];
    QJsonObject o;
    o["id"]      = i;
    o["type"]    = a.typeStr;
    o["name"]    = a.nameStr;
    o["account"] = a.accountStr;
    o["pwd"]     = a.pwdStr;
    o["others"]  = a.othersStr;
    // 新增两个状态位
    o["isModified"] = a.IsModified();
    o["isDeleted"]  = a.IsDeleted();
    arr.append(o);
  }

  QJsonObject root;
  root["count"]    = m_accountStorage.size();
  root["accounts"] = arr;
  return QJsonDocument(root).toJson(QJsonDocument::Compact).toStdString();
}


// ---------- POST /api/setkey：设置 AES 密钥并重新加载账户 ----------

bool PasswordBookServer::onPostSetKey(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const QString aesKey = doc.object().value("key").toString();
  if (aesKey.isEmpty()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"empty key"})", "application/json");
    return false;
  }

  // 注意：setAccounts 内部已经加锁，这里不能再加 QMutexLocker，否则死锁
  const bool ok = setAccounts(aesKey);
  if (!ok) {
    res.status = 401;
    res.set_content(R"({"ok":false,"error":"invalid key or corrupted file."})", "application/json");
    return false;
  }

  QString token;
  {
    QMutexLocker lock(&m_tokenMutex);
    if (m_token.isEmpty()) {
      m_token = QUuid::createUuid().toString(QUuid::WithoutBraces);;
    }
    token = m_token;
  }

  QJsonObject o;
  o["ok"] = true;
  o["token"] = token;
  res.set_content(QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString(), "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- POST /api/update：保存某一条 ----------

bool PasswordBookServer::onPostUpdate(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;

  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const QJsonObject o  = doc.object();
  const int         id = o.value("id").toInt(-1);

  QMutexLocker lock(&m_accountsMutex);
  if (id < 0 || id >= m_accountStorage.size()) {
    res.status = 404;
    res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
    return false;
  }

  AccountInfo& a = m_accountStorage[id];
  a.typeStr    = o.value("type").toString();
  a.nameStr    = o.value("name").toString();
  a.accountStr = o.value("account").toString();
  a.pwdStr     = o.value("pwd").toString();
  a.othersStr  = o.value("others").toString();
  // 这里只是从html界面写入到内存中, "isModified"字段比较的是磁盘中的数据和内存的数据是否一致
  // 因此无需修改isModified字段

  res.set_content(R"({"ok":true})", "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- POST /api/delete：标记删除 ----------

bool PasswordBookServer::onPostDelete(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;

  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const int id = doc.object().value("id").toInt(-1);

  QMutexLocker lock(&m_accountsMutex);
  if (id < 0 || id >= m_accountStorage.size()) {
    res.status = 404;
    res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
    return false;
  }

  m_accountStorage[id].SetDeleted(true);
  m_accountStorage[id].SetModified(true);

  res.set_content(R"({"ok":true})", "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- POST /api/restore：取消删除标记 ----------

bool PasswordBookServer::onPostRestore(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;

  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const int id = doc.object().value("id").toInt(-1);

  QMutexLocker lock(&m_accountsMutex);
  if (id < 0 || id >= m_accountStorage.size()) {
    res.status = 404;
    res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
    return false;
  }

  m_accountStorage[id].SetDeleted(false);
  m_accountStorage[id].SetModified(true);   // 恢复也是内存修改，需要落盘

  res.set_content(R"({"ok":true})", "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- POST /api/add：尾部新增一条空记录 ----------

bool PasswordBookServer::onPostAdd(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;

  QMutexLocker lock(&m_accountsMutex);
  AccountInfo a;
  a.SetDeleted(false);
  a.SetModified(false);
  m_accountStorage.append(a);
  const int newId = m_accountStorage.size() - 1;

  QJsonObject o;
  o["ok"] = true;
  o["id"] = newId;
  res.set_content(QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString(), "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- POST /api/commit：写入磁盘 ----------
bool PasswordBookServer::onPostCommit(const httplib::Request& req, httplib::Response& res) {
  if (!CheckToken(req, res)) return false;
  if (!m_accountStorage.IsValid()) {
    res.set_content(R"({"ok":false,"error":"reject write. avoid encrypte file get corrupt by wrong aes key"})", "application/json");
    return false;
  }
  QMutexLocker lock(&m_accountsMutex);
  const bool bSucceed = m_accountStorage.CommitChangesToEncryptedCsvFile();
  if (!bSucceed) {
    res.set_content(R"({"ok":false,"error":"save file failed"})", "application/json");
    return false;
  }
  res.set_content(R"({"ok":true})", "application/json");
  if (res.status == -1) res.status = 200;
  return true;
}

// ---------- 局域网地址 ----------

QStringList PasswordBookServer::lanAddresses() {
  QStringList out;
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    const auto flags = iface.flags();
    if (!(flags & QNetworkInterface::IsUp))        continue;
    if (!(flags & QNetworkInterface::IsRunning))   continue;
    if (flags & QNetworkInterface::IsLoopBack)     continue;
    for (const QNetworkAddressEntry& e : iface.addressEntries()) {
      const QHostAddress ip = e.ip();
      if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
        out << ip.toString();
      }
    }
  }
  out.removeDuplicates();
  return out;
}

// ---------- 内嵌 HTML ----------
namespace {
QByteArray LoadIndexHtml() {
  QFile f(QStringLiteral(":/pwd/app/PASSWORD_BOOK_INDEX"));
  if (!f.open(QIODevice::ReadOnly)) {
    qWarning("Failed to open embedded index.html: %s", qPrintable(f.fileName()));
    return QByteArrayLiteral(
        "<!DOCTYPE html><html><body>"
        "<h1>Password Server</h1><p>index.html not found in resources.</p>"
        "</body></html>");
  }
  return f.readAll();
}
}  // namespace

QByteArray PasswordBookServer::indexHtml() {
  static const QByteArray cached = LoadIndexHtml();
  return cached;
}

// #include <QDesktopServices>
// #include <QCoreApplication>
// #include "SystemPath.h"

// int main(int argc, char* argv[]) {
//   if (!SystemPath::initDirectory()) {
//     return -1;
//   }

//   QCoreApplication app{argc, argv};
//   PasswordBookServer pwdServer;
//   pwdServer.setAccounts(/*aesKey*/ "");
//   if (!pwdServer.start(PasswordBookServer::DEFAULT_PORT)) {
//     qDebug("启动端口失败, 可能被占用");
//     return -1;
//   }
//   qDebug("电脑访问: %s", qPrintable(pwdServer.urlAddress()));
//   for (const QString& ip : PasswordBookServer::lanAddresses()) {
//     qDebug("手机访问: http://%s:%d", qPrintable(ip), pwdServer.port());
//   }
//   return app.exec();
// }