#include "MovieWallServer.h"
#include "httplib.h"
#include "PathTool.h"
#include "Logger.h"
#include "RateHelper.h"

#include <QThread>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>

constexpr const char* MovieWallServer::DEFAULT_MOVIE_WALL_HOST;
constexpr int MovieWallServer::DEFAULT_MOVIE_WALL_PORT;

MovieWallServer::MovieWallServer(QObject* parent, bool bIncludeSubdirectory)
  : QObject{parent}, m_bSubdirectories{bIncludeSubdirectory}, m_thumbCache(128) {
  mFolderIterator.setWrapEnabled(false);
}

MovieWallServer::~MovieWallServer() {
  stop();
}

bool MovieWallServer::setRootPath(const QString& rootPath) {
  if (!rootPath.isEmpty() && !QFileInfo(rootPath).isDir()) {
    return false;
  }

  bool bSub{false};
  {
    QMutexLocker lock(&m_sceneMutex);
    bSub = m_bSubdirectories;
  }

  SceneInfoList newScenes = SceneHelper::GetScnsLstFromPath(rootPath, bSub);
  {
    QMutexLocker lock(&m_sceneMutex);
    mRootPath       = rootPath;
    m_sceneElements = std::move(newScenes);
  }
  return true;
}

bool MovieWallServer::onIteratorToAnotherFolderCore(bool isNext) {
  QString snapshotPath;
  {
    QMutexLocker lock(&m_sceneMutex);
    snapshotPath = mRootPath;
  }

  QString parentPath;
  QString curDirName = PathTool::GetPrepathAndFileName(snapshotPath, parentPath);

  const QString& newDir = isNext ? mFolderIterator.next(parentPath, curDirName) : mFolderIterator.last(parentPath, curDirName);
  if (newDir.isEmpty()) {
    LOG_D("skip empty dir[%s]", qPrintable(parentPath));
    return false;
  }
  if (curDirName == newDir) {
    LOG_D("skip same dir[%s/%s]", qPrintable(parentPath), qPrintable(curDirName));
    return false;
  }
  const QString newPath = PathTool::Path2Join(parentPath, newDir);
  return setRootPath(newPath);
}

bool MovieWallServer::onIncludeSubdirectoryChanged(bool bIncludeSubdirectory) {
  {
    QMutexLocker lock(&m_sceneMutex);
    if (m_bSubdirectories == bIncludeSubdirectory) {
      return false;
    }
  }

  QString rootPath;
  {
    QMutexLocker lock(&m_sceneMutex);
    rootPath = mRootPath;
  }
  SceneInfoList newScenes = SceneHelper::GetScnsLstFromPath(rootPath, bIncludeSubdirectory);  // 锁外
  {
    QMutexLocker lock(&m_sceneMutex);
    m_bSubdirectories = bIncludeSubdirectory;
    m_sceneElements   = std::move(newScenes);
  }
  return true;
}

constexpr const char* THUMB_IMAGE_TEMPLATE{R"(/thumb/%1/%2)"};                  // "/thumb/mRootPath/rel/imgName/IndexInm_sceneElements" 缩略图路由
constexpr const char* THUMB_IMAGE_PATTERN{R"(/thumb/(.*?)/(\d+)$)"};            // non-gready
constexpr const char* DETAIL_IMAGE_TEMPLATE{R"(/detailimg/%1/%2/%3)"};          // "/detailimg/mRootPath/rel/imgName/IndexInm_sceneElements/IndexInImagesList" 主图路由
constexpr const char* DETAIL_IMAGE_PATTERN{R"(/detailimg/(.*?)/(\d+)/(\d+)$)"}; // non-gready

bool MovieWallServer::start(quint16 port) {
  if (m_server != nullptr) {
    return true;
  }
  httplib::Server* svr = new (std::nothrow) httplib::Server();
  m_server = svr;
  if (!svr->bind_to_port(DEFAULT_MOVIE_WALL_HOST, port)) {
    LOG_W("MovieWallServer: cannot bind port[%d]", port);
    stop();
    return false;
  }
  m_port = port;

  svr->Get("/", [](const httplib::Request&, httplib::Response& res) {
    const QByteArray html = indexHtml();
    res.set_content(std::string(html.constData(), static_cast<size_t>(html.size())), "text/html; charset=utf-8");
    res.set_header("Cache-Control", "no-cache");
  });

  svr->Get("/api/movies", [this](const httplib::Request&, httplib::Response& res) {
    res.set_content(buildMoviesJson(), "application/json; charset=utf-8");
  });

  svr->Get(THUMB_IMAGE_PATTERN, [this](const httplib::Request& req, httplib::Response& res) {
    PostApiThumbIdCallback(req, res);
  });

  svr->Post("/api/rate", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiRateCallback(req, res);
  });

  svr->Post("/api/openvideo", [this](const httplib::Request& req, httplib::Response& res){
    PostApiOpenVideoCallback(req, res);
  });

  svr->Get(R"(/api/detail/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiDetailCallback(req, res);
  });

  svr->Get(DETAIL_IMAGE_PATTERN, [this](const httplib::Request& req, httplib::Response& res) {
    PostApiDetailImgCallback(req, res);
  });

  svr->Post("/api/setsubdirectories", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiSetSubdirectories(req, res);
  });

  svr->Post("/api/setrootpath", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiSetRootPath(req, res);
  });

  svr->Post("/api/navigatefolder", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiNavigateFolder(req, res);
  });

  m_thread = std::thread([svr]() { svr->listen_after_bind(); });
  return true;
}

void MovieWallServer::stop() {
  if (m_server) static_cast<httplib::Server*>(m_server)->stop();
  if (m_thread.joinable()) m_thread.join();
  delete static_cast<httplib::Server*>(m_server);
  m_server = nullptr;
}

bool MovieWallServer::PostApiThumbIdCallback(const httplib::Request& req, httplib::Response& res) {
  const QString thumbnailImgFullPath = QString::fromStdString(req.matches[1].str());
  const int eleId = std::stoi(req.matches[2].str());

  const QImage img = getThumbnail(thumbnailImgFullPath, eleId);
  if (img.isNull()) {
    res.status = 404;
    return false;
  }

  QByteArray ba;
  QBuffer buf(&ba);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, "JPEG");

  res.set_content(std::string(ba.constData(), static_cast<size_t>(ba.size())), "image/jpeg");
  res.set_header("Cache-Control", "max-age=86400");
  return true;
}

bool MovieWallServer::PostApiRateCallback(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(
      QByteArray::fromStdString(req.body), &perr);

  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})",
                    "application/json");
    return false;
  }

  const QJsonObject obj = doc.object();
  const int id      = obj.value("id").toInt(-1);
  const int newRate = obj.value("rate").toInt(-1);

  if (newRate < 0 || newRate > 10) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"rate must be 0-10"})", "application/json");
    return false;
  }

  // 读内存
  QString eleBaseName;
  QString scnAbsFilePath, jsonAbsFilePath;
  QString beforePath;
  {
    QMutexLocker lock(&m_sceneMutex);
    if (id < 0 || id >= m_sceneElements.size()) {
      res.status = 404;
      res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
      return false;
    }
    const SceneInfo& item = m_sceneElements[id];
    scnAbsFilePath = item.GetSceneFullPath(mRootPath);
    eleBaseName = item.name;
    jsonAbsFilePath = item.GetJsonAbsPath(mRootPath);
    beforePath = mRootPath;
  }

  // I/O 读写
  const bool bScnUpdatedOk = SceneHelper::UpdateNameWithNewRate(scnAbsFilePath, eleBaseName, newRate);
  const bool bJsonUpdatedOk = RateHelper::SetFileRate(jsonAbsFilePath, newRate);
  const bool bSucceed {bScnUpdatedOk && bJsonUpdatedOk};
  // 写内存
  if (bSucceed) {
    QMutexLocker lock(&m_sceneMutex);
    if (beforePath == mRootPath && 0 <= id && id < m_sceneElements.size()) {
      m_sceneElements[id].rate = newRate;
    }
  }

  LOG_D("[rate] id:%d, name:%s, newRate:%d, scnResult:%d, jsonResult:%d", id, qPrintable(eleBaseName), newRate, bScnUpdatedOk, bJsonUpdatedOk);
  if (bSucceed) {
    res.set_content(R"({"ok":true})", "application/json");
  } else {
    res.set_content(R"({"ok":false,"error":"scene file or json file updated failed"})", "application/json");
  }
  return bSucceed;
}

bool MovieWallServer::PostApiOpenVideoCallback(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const int id = doc.object().value("id").toInt(-1);

  QString path, name;
  {
    QMutexLocker lock(&m_sceneMutex);
    if (id < 0 || id >= m_sceneElements.size()) {
      res.status = 404;
      res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
      return false;
    }
    const SceneInfo& item = m_sceneElements[id];
    path = item.GetVideoFullPath(mRootPath);
    name = item.name;
  }

  const QFileInfo fi(path);
  if (path.isEmpty() || !fi.isFile()) {
    LOG_D("[video] not found, id:%d, path:%s", id, qPrintable(path));
    res.status = 404;
    res.set_content(R"({"ok":false,"error":"video not found"})", "application/json");
    return false;
  }

  const QString absPath = fi.absoluteFilePath();

  // QDesktopServices 必须在主线程调，HTTP handler 在工作线程，排队过去
  QMetaObject::invokeMethod(this, [id, name, absPath]() {
    const bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(absPath));
    LOG_D("[video] id:%d, name:%s, path:%s, launch:%d", id, qPrintable(name), qPrintable(absPath), ok);
  }, Qt::QueuedConnection);

  res.set_content(R"({"ok":true})", "application/json");
  return true;
}

QJsonObject MovieWallServer::GetQJsonObject(int eleId, bool bIsDetail) const {
  QJsonObject o;
  const SceneInfo& s = m_sceneElements[eleId];
  o["id"]       = eleId;
  o["name"]     = s.name;
  o["rate"]     = s.rate;
  o["vidSize"]  = s.vidSize;
  o["duration"] = s.duration;
  o["rel2Name"] = s.GetRel2Name();
  if (bIsDetail) {
    const QStringList detailImages = s.GetImagesAbsPathList(mRootPath);
    QJsonArray imgs;
    for (int indexInImgs = 0; indexInImgs < detailImages.size(); ++indexInImgs) {
      const QString& pth = detailImages[indexInImgs];
      imgs.append(QString(DETAIL_IMAGE_TEMPLATE).arg(pth).arg(eleId).arg(indexInImgs));
    }
    o["imgs"] = imgs;
  } else {
    const QString thumbImg = s.GetThumbnailImageAbsPath(mRootPath);
    o["thumbUrl"] = QString(THUMB_IMAGE_TEMPLATE).arg(thumbImg).arg(eleId);
  }
  return o;
}

bool MovieWallServer::PostApiDetailCallback(const httplib::Request& req, httplib::Response& res) {
  const int id = std::stoi(req.matches[1].str());

  QJsonObject o;
  {
    QMutexLocker lock(&m_sceneMutex);
    if (id < 0 || id >= m_sceneElements.size()) {
      res.status = 404;
      res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
      return false;
    }
    o = GetQJsonObject(id, true);
  }

  res.set_content(QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString(), "application/json; charset=utf-8");
  return true;
}

bool MovieWallServer::PostApiDetailImgCallback(const httplib::Request& req, httplib::Response& res) {
  const QString detailImgFullPath = QString::fromStdString(req.matches[1].str());
  const int eleId  = std::stoi(req.matches[2].str());
  const int imgIdx = std::stoi(req.matches[3].str());

  const QString path = resolveImage(eleId, imgIdx);
  QImage img;
  if (!GetQImage(path, img, false, true)) {
    res.status = 404;
    return false;
  }

  QByteArray ba;
  QBuffer buf(&ba);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, "JPEG");

  res.set_content(std::string(ba.constData(), static_cast<size_t>(ba.size())), "image/jpeg");
  res.set_header("Cache-Control", "max-age=3600");
  return true;
}

bool MovieWallServer::PostApiSetSubdirectories(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const bool value = doc.object().value("value").toBool(false);

  const bool changed = invokeOnMainThread([this, value]() {
    return onIncludeSubdirectoryChanged(value);
  });

  res.set_content(changed ? R"({"ok":true})" : R"({"ok":false})", "application/json");
  return true;
}

bool MovieWallServer::PostApiSetRootPath(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const QString path = doc.object().value("path").toString();
  if (path.isEmpty()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"empty path"})", "application/json");
    return false;
  }

  const bool changed = invokeOnMainThread([this, path]() {
    return setRootPath(path);
  });
  res.set_content(changed ? R"({"ok":true})" : R"({"ok":false})", "application/json");
  return true;
}

bool MovieWallServer::PostApiNavigateFolder(const httplib::Request& req, httplib::Response& res) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const bool isNext = doc.object().value("isNext").toBool(false);

  // onIteratorToAnotherFolderCore 内部扫盘 + mFolderIterator 有状态，排队到主线程
  const bool changed = invokeOnMainThread([this, isNext]() {
    return onIteratorToAnotherFolderCore(isNext);
  });

  res.set_content(changed ? R"({"ok":true})" : R"({"ok":false})", "application/json");
  return true;
}

QString MovieWallServer::resolveImage(int id, int imgIdx) const {
  QMutexLocker lock(&m_sceneMutex);
  if (id < 0 || id >= m_sceneElements.size()) return {};
  return m_sceneElements[id].GetImageAtIndex(mRootPath, imgIdx);
}

QString MovieWallServer::resolveThumbnailImage(int id) const {
  QMutexLocker lock(&m_sceneMutex);
  if (id < 0 || id >= m_sceneElements.size()) return {};
  return m_sceneElements[id].GetThumbnailImageAbsPath(mRootPath);
}

bool MovieWallServer::invokeOnMainThread(std::function<bool()> fn) {
  if (QThread::currentThread() == this->thread()) {
    return fn();
  }
  bool result = false;
  QMetaObject::invokeMethod(this, [&fn, &result]() { result = fn(); }, Qt::BlockingQueuedConnection);
  return result;
}

std::string MovieWallServer::buildMoviesJson() const {
  QMutexLocker lock(&m_sceneMutex);

  QJsonObject root;
  root["rootPath"]       = mRootPath;
  root["subdirectories"] = m_bSubdirectories;

  QJsonArray arr;
  for (int id = 0; id < m_sceneElements.size(); ++id) {
    arr.append(GetQJsonObject(id, false));   // GetQJsonObject 要求调用方已持锁
  }
  root["movies"] = arr;

  return QJsonDocument(root).toJson(QJsonDocument::Compact).toStdString();
}

constexpr int DETAIL_MAIN_IMAGE_MAX_SIZE{1600};
constexpr int PREFEREED_DEFAULT_IMAGE_WIDTH{480}, PREFEREED_DEFAULT_IMAGE_HEIGHT{320};

bool MovieWallServer::GetQImage(const QString& path, QImage& img, bool enablePlaceholderImage, bool isDetailImage) {
  const bool bIsImageNotExist{path.isEmpty() || !QFileInfo::exists(path)};
  if (bIsImageNotExist) {
    if (!enablePlaceholderImage) {
      return false; // 404
    } else {
      // 占位图： 灰底 + 名字
      static QImage notExistImage{[](){
        QImage img{PREFEREED_DEFAULT_IMAGE_WIDTH, PREFEREED_DEFAULT_IMAGE_HEIGHT, QImage::Format_RGB32};
        img.fill(QColor(50, 50, 50));
        return img;
      }()};
      img = notExistImage;
      return true;
    }
  }

  QImageReader reader(path);
  reader.setAutoTransform(true);
  const QSize orig = reader.size();
  if (orig.isValid()) {
    if (isDetailImage) {
      const bool bIsLargeImage{qMax(orig.width(), orig.height()) > DETAIL_MAIN_IMAGE_MAX_SIZE};
      // 详情图 不裁剪，但限制长边到 1600，避免 4K 原图太大
      if (bIsLargeImage) {
        reader.setScaledSize(orig.scaled(DETAIL_MAIN_IMAGE_MAX_SIZE, DETAIL_MAIN_IMAGE_MAX_SIZE, Qt::KeepAspectRatio));
      }
    } else {
      // 普通图 墙纸部分裁剪到一个小尺寸
      reader.setScaledSize(orig.scaled(PREFEREED_DEFAULT_IMAGE_WIDTH, PREFEREED_DEFAULT_IMAGE_HEIGHT, Qt::KeepAspectRatio));
    }
  }
  img = reader.read();
  return true;
}

QImage MovieWallServer::getThumbnail(const QString& thumbImgFullPath, const int eleId) const {
  const QString path{resolveThumbnailImage(eleId)};
  if (path.isEmpty()) {
    LOG_W("eleId[%d] out of range or no image", eleId);
    return {};
  }

  {
    QMutexLocker lock(&m_cacheMutex);
    if (const QImage* cached = m_thumbCache.object(thumbImgFullPath)) {
      return *cached;
    }
  }

  QImage result;
  GetQImage(path, result, true, false);

  {
    QMutexLocker lock(&m_cacheMutex);
    m_thumbCache.insert(thumbImgFullPath, new QImage(result));
  }
  return result;
}

QByteArray MovieWallServer::indexHtml() {
  // R"HTML()HTML"
  // R"()"
  static const QByteArray cached = []() {
    QFile f(QStringLiteral(":img/MOVIE_WALL"));
    if (!f.open(QIODevice::ReadOnly)) {
      LOG_W("failed to open embedded index.html");
      return QByteArrayLiteral(
          "<!DOCTYPE html><html><body>"
          "<h1>Movie Wall</h1><p>index.html not found in resources.</p>"
          "</body></html>");
    }
    return f.readAll();
  }();
  return cached;
}