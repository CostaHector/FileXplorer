#include "MovieWallServer.h"
#include "httplib.h"
#include "PathTool.h"
#include "Logger.h"
#include "RateHelper.h"
#include "PublicMacro.h"

#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QPainter>

constexpr const char* MovieWallServer::DEFAULT_MOVIE_WALL_HOST;
constexpr int MovieWallServer::DEFAULT_MOVIE_WALL_PORT;

MovieWallServer::MovieWallServer(QObject* parent)
  : QObject{parent}, m_thumbCache(128) {
}

MovieWallServer::~MovieWallServer() {
  stop();
}

void MovieWallServer::setRootPath(const QString rootPath, SceneInfoList* pScenesList) {
  if (mRootPath == rootPath) {
    return;
  }

  {
    QMutexLocker lock(&m_cacheMutex);
    m_thumbCache.clear();
  }
  {
    QMutexLocker lock(&m_sceneMutex);
    mRootPath = rootPath;
    _mPSceneElements = pScenesList;
  }

  m_dataVersion.fetch_add(1, std::memory_order_release);
}

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
    res.set_content(indexHtml(), "text/html; charset=utf-8");
    res.set_header("Cache-Control", "no-cache");
  });

  svr->Get("/api/movies", [this](const httplib::Request&, httplib::Response& res) {
    res.set_content(buildMoviesJson(), "application/json; charset=utf-8");
  });

  svr->Get(R"(/thumb/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
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

  svr->Get(R"(/detailimg/(\d+)/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
    PostApiDetailImgCallback(req, res);
  });

  m_thread = std::thread([svr]() { svr->listen_after_bind(); });
  return true;
}

bool MovieWallServer::PostApiThumbIdCallback(const httplib::Request& req, httplib::Response& res) {
  const int id = std::stoi(req.matches[1].str());
  const QImage img = getThumbnail(id);
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
  CHECK_NULLPTR_RETURN_FALSE(_mPSceneElements);

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
    if (id < 0 || id >= (*_mPSceneElements).size()) {
      res.status = 404;
      res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
      return false;
    }
    const SceneInfo& item = (*_mPSceneElements)[id];
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
    if (beforePath == mRootPath && _mPSceneElements != nullptr && 0 <= id && id < _mPSceneElements->size()) {
      (*_mPSceneElements)[id].rate = newRate;
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
  CHECK_NULLPTR_RETURN_FALSE(_mPSceneElements);

  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(
      QByteArray::fromStdString(req.body), &perr);
  if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
    res.status = 400;
    res.set_content(R"({"ok":false,"error":"invalid json"})", "application/json");
    return false;
  }

  const int id = doc.object().value("id").toInt(-1);

  QString path, name;
  {
    QMutexLocker lock(&m_sceneMutex);
    if (id < 0 || id >= (*_mPSceneElements).size()) {
      res.status = 404;
      res.set_content(R"({"ok":false,"error":"id out of range"})", "application/json");
      return false;
    }
    const SceneInfo& item = (*_mPSceneElements)[id];
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


QJsonObject MovieWallServer::GetQJsonObject(int id, bool bIsDetail) const {
  CHECK_FALSE_RETURN_VAL(_mPSceneElements != nullptr, {});
  QJsonObject o;
  const SceneInfo& s = (*_mPSceneElements)[id];
  o["id"]       = id;
  o["name"]     = s.name;
  o["rate"]     = s.rate;
  o["vidSize"]  = s.vidSize;
  o["duration"] = s.duration;
  if (bIsDetail) {
    QJsonArray imgs;
    for (int i = 0; i < s.imgs.size(); ++i) {
      imgs.append(QString("/detailimg/%1/%2?v=%3").arg(id).arg(i).arg(m_dataVersion));
    }
    o["imgs"] = imgs;
  } else {
    o["thumbUrl"] = QString("/thumb/%1?v=%2").arg(id).arg(m_dataVersion);
  }
  return o;
}

bool MovieWallServer::PostApiDetailCallback(const httplib::Request& req, httplib::Response& res) {
  CHECK_NULLPTR_RETURN_FALSE(_mPSceneElements);

  const int id = std::stoi(req.matches[1].str());

  QJsonObject o;
  {
    QMutexLocker lock(&m_sceneMutex);
    if (id < 0 || id >= (*_mPSceneElements).size()) {
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
  CHECK_NULLPTR_RETURN_FALSE(_mPSceneElements);

  const int id     = std::stoi(req.matches[1].str());
  const int imgIdx = std::stoi(req.matches[2].str());

  const QString path = resolveImage(id, imgIdx);
  QImage img;
  if (!GetQImage(path, img, false, true)) {
    res.status = 404;
    return false;
  }

  QByteArray ba;
  QBuffer buf(&ba);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, "JPEG", 88);

  res.set_content(std::string(ba.constData(), static_cast<size_t>(ba.size())), "image/jpeg");
  res.set_header("Cache-Control", "max-age=3600");
  return true;
}

void MovieWallServer::stop() {
  if (m_server) static_cast<httplib::Server*>(m_server)->stop();
  if (m_thread.joinable()) m_thread.join();
  delete static_cast<httplib::Server*>(m_server);
  m_server = nullptr;
}

QString MovieWallServer::resolveVideo(int id) const {
  CHECK_FALSE_RETURN_VAL(_mPSceneElements != nullptr, "");
  if (id < 0 || id >= (*_mPSceneElements).size()) return {};
  return (*_mPSceneElements)[id].GetVideoFullPath(mRootPath);
}

QString MovieWallServer::resolveImage(int id, int imgIdx) const {
  CHECK_FALSE_RETURN_VAL(_mPSceneElements != nullptr, "");
  if (id < 0 || id >= (*_mPSceneElements).size()) return {};
  const SceneInfo& s = (*_mPSceneElements)[id];
  if (imgIdx < 0 || imgIdx >= s.imgs.size()) return {};

  const QStringList absList = s.GetImagesAbsPathList(mRootPath);
  if (imgIdx >= absList.size()) return {};
  return absList[imgIdx];
}

std::string MovieWallServer::buildMoviesJson() const {
  CHECK_FALSE_RETURN_VAL(_mPSceneElements != nullptr, "[]");
  QJsonArray arr;
  {
    QMutexLocker lock(&m_sceneMutex);
    for (int id = 0; id < (*_mPSceneElements).size(); ++id) {
      QJsonObject o = GetQJsonObject(id, false);
      arr.append(o);
    }
  }
  return QJsonDocument(arr).toJson(QJsonDocument::Compact).toStdString();
}

constexpr int DETAIL_MAIN_IMAGE_MAX_SIZE{1600};
constexpr int PREFEREED_DEFAULT_IMAGE_WIDTH{480}, PREFEREED_DEFAULT_IMAGE_HEIGHT{320};

bool MovieWallServer::GetQImage(const QString& path, QImage& img, bool enablePlaceholderImage, bool isDetailImage) {
  const bool bIsImageNotExist{path.isEmpty() || !QFileInfo::exists(path)};
  if (bIsImageNotExist && !enablePlaceholderImage) {
    return false; // 404
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
  if (enablePlaceholderImage && img.isNull()) {
    // 占位图： 灰底 + 名字
    LOG_D("image path[%s] read failed. use default", qPrintable(path));

    img = QImage(PREFEREED_DEFAULT_IMAGE_WIDTH, PREFEREED_DEFAULT_IMAGE_HEIGHT, QImage::Format_RGB32);
    img.fill(QColor(50, 50, 50));
    QPainter p(&img);
    p.setPen(Qt::gray);

    p.drawText(img.rect(), Qt::AlignCenter | Qt::TextWordWrap, PathTool::fileName(path));
    p.end();
  }
  return true;
}

QImage MovieWallServer::getThumbnail(int id) const {
  CHECK_FALSE_RETURN_VAL(_mPSceneElements != nullptr, {});
  if (id < 0 || id >= (*_mPSceneElements).size()) {
    LOG_W("id[%d] Out of range", id);
    return {};
  }

  {
    QMutexLocker lock(&m_cacheMutex);
    if (const QImage* cached = m_thumbCache.object(id)) {
      return *cached;
    }
  }

  const QString path = resolveImage(id, 0);
  QImage result;
  GetQImage(path, result, true, false);

  {
    QMutexLocker lock(&m_cacheMutex);
    m_thumbCache.insert(id, new QImage(result));
  }
  return result;
}

const char* MovieWallServer::indexHtml() {
  return R"HTML(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Movie Wall</title>
<style>
* { box-sizing: border-box; }
body { margin: 0; padding: 16px; background: #1a1a1a; color: #eee;
       font-family: system-ui, sans-serif; }
h1 { font-size: 20px; margin: 0 0 16px; }

/* 每列5个 */
<!--.poster-wall {
  display: grid;
  grid-template-columns: repeat(5, 1fr);
  gap: 16px;
}-->
/* Flow: Left To Right */
.poster-wall {
  display: grid;
  /* 每列至少 280px，剩余空间由 1fr 均分；放不下就自动换行 */
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 16px;
}

.poster {
  background: #2a2a2a;
  border-radius: 8px;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

/* 海报图片：宽度撑满卡片，高度按原图比例，不裁剪 */
.poster img {
  display: block;
  width: 100%;
  height: auto;
  background: #111;            /* 图片加载前的占位底色，避免闪白 */
  cursor: pointer;             /* 提示可点击（双击开视频） */
  user-select: none;           /* 双击时不选中图片 */
  -webkit-user-drag: none;     /* 双击时不触发拖拽 */
  transition: filter 0.15s;    /* 悬停提亮的过渡 */
}

.poster img:hover {
  filter: brightness(1.08);    /* 轻微提亮，提示可交互 */
}

/* 中间层：时长 | 文件大小 | 星级 */
.poster .meta {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 6px;
  padding: 6px 8px;
  background: #222;
  border-top: 1px solid #333;
  border-bottom: 1px solid #333;
  font-size: 12px;
  min-height: 28px;
}

/* 时长：固定 8 字符，不伸缩 */
.poster .meta .duration {
  flex: 0 0 auto;
  color: #aaa;
  white-space: nowrap;
  font-variant-numeric: tabular-nums;   /* 数字等宽，XX:XX:XX 不会抖动 */
  letter-spacing: 0.2px;
}

/* 文件大小：唯一可伸缩的字段，太长省略 */
.poster .meta .size {
  flex: 1 1 auto;
  min-width: 0;
  color: #888;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  text-align: center;
}

/* 评分区：星星 + 数值，整体不压缩 */
.poster .meta .rating {
  flex: 0 0 auto;
  display: flex;
  align-items: center;
  gap: 4px;
  white-space: nowrap;
}

/* 星星 */
.poster .meta .stars {
  font-size: 11px;
  letter-spacing: 0.5px;
  cursor: pointer;
  user-select: none;
}

.poster .meta .stars .star { display: inline-block; transition: transform 0.1s; }
.poster .meta .stars .star:hover { transform: scale(1.25); }
.poster .meta .stars .on  { color: #f5c518; }
.poster .meta .stars .off { color: #555; }

/* 数值：等宽、右对齐，0-9 和 10 占位一致，星星不会左右跳 */
.poster .meta .score {
  font-size: 12px;
  font-weight: 600;
  min-width: 1.6em;
  text-align: right;
  font-variant-numeric: tabular-nums;
}
.poster .meta .score.none { color: #666; }   /* 0 */
.poster .meta .score.low  { color: #e05c5c; } /* 1-4 */
.poster .meta .score.mid  { color: #e0a63c; } /* 5-7 */
.poster .meta .score.high { color: #4caf50; } /* 8-10 */

/* 名称层：自动换行，不省略 */
.poster .name {
  padding: 8px;
  font-size: 13px;
  line-height: 1.4;
  white-space: normal;        /* 覆盖 nowrap，允许换行 */
  overflow-wrap: anywhere;    /* 长英文单词 / 无空格字符串也能断行 */
  word-break: break-word;     /* 旧浏览器兜底 */
  cursor: pointer;             /* 提示可点击播放 */
}
.poster .name:hover {
  color: #fff;                 /* 悬停变白，暗示可交互 */
}

/* ---------- 详情抽屉 + 缩略图轮盘 ---------- */
.detail-backdrop {
  position: fixed; inset: 0; z-index: 900;
  background: rgba(0, 0, 0, 0.6);
  opacity: 0; pointer-events: none;
  transition: opacity 0.25s ease;
}
.detail-backdrop.open {
  opacity: 1; pointer-events: auto;
}

.detail-drawer {
  position: fixed; left: 0; right: 0; bottom: 0; z-index: 901;
  height: 80vh;                /* 固定高度，不再用 max-height */
  background: #202020;
  border-radius: 16px 16px 0 0;
  box-shadow: 0 -8px 32px rgba(0, 0, 0, 0.5);
  transform: translateY(100%);
  transition: transform 0.3s cubic-bezier(0.32, 0.72, 0, 1);
  display: flex; flex-direction: column;
  overflow: hidden;            /* 防止圆角处内容溢出 */
}
.detail-drawer.open { transform: translateY(0); }

/* 顶部拖拽把手，纯视觉提示，不实现拖拽 */
.detail-drawer .handle {
  width: 40px; height: 4px; margin: 8px auto 0;
  background: #555; border-radius: 2px; flex: 0 0 auto;
}

.detail-drawer .head {
  display: flex; align-items: center; justify-content: space-between;
  padding: 12px 20px 8px; flex: 0 0 auto;
}
.detail-drawer .head .title {
  font-size: 18px; font-weight: 600; color: #eee;
  overflow: hidden; text-overflow: ellipsis; white-space: nowrap;
  cursor: pointer;             /* 提示可点击播放 */
}
.detail-drawer .head .title:hover {
  color: #fff;
}
.detail-drawer .head .close-btn {
  background: none; border: none; color: #aaa; font-size: 26px;
  cursor: pointer; line-height: 1; padding: 0 4px;
}
.detail-drawer .head .close-btn:hover { color: #fff; }

.detail-drawer .sub {
  padding: 0 20px 12px; color: #888; font-size: 13px;
  display: flex; gap: 16px; flex: 0 0 auto;
}

/* 中间层，负责让 hero 在抽屉内收缩 */
.detail-drawer .gallery {
  flex: 1 1 auto;
  display: flex;
  flex-direction: column;
  min-height: 0;      /* 关键：不设 0，hero 的 max-height 会被忽略 */
  overflow: hidden;
}

/* 索引行：‹  3 / 7  › */
.detail-drawer .index-bar {
  flex: 0 0 auto;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  padding: 4px 20px 8px;
  color: #aaa;
  font-size: 13px;
  font-variant-numeric: tabular-nums;
}
.detail-drawer .index-bar .nav-btn {
  background: #2e2e2e;
  border: none;
  color: #ddd;
  width: 28px;
  height: 28px;
  border-radius: 50%;
  font-size: 18px;
  line-height: 1;
  cursor: pointer;
  transition: background 0.15s;
}
.detail-drawer .index-bar .nav-btn:hover:not(:disabled) { background: #3a3a3a; }
.detail-drawer .index-bar .nav-btn:disabled {
  opacity: 0.3;
  cursor: not-allowed;
}

/* 大图区 */
.detail-drawer .hero {
  flex: 1 1 auto;
  min-height: 0;               /* 关键：允许收缩，不再写死 200px */
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0 20px 8px;
  overflow: hidden;
}
.detail-drawer .hero img {
  max-width: 100%;
  max-height: 100%;            /* 相对 hero，而不是 50vh */
  object-fit: contain;
  border-radius: 8px;
  background: #111;
}

/* 缩略图轮盘 */
.detail-drawer .thumbstrip {
  flex: 0 0 auto;
  display: flex;
  gap: 8px;
  padding: 8px 20px 20px;
  overflow-x: auto;
  scroll-behavior: smooth;
}
.detail-drawer .thumbstrip::-webkit-scrollbar { height: 6px; }
.detail-drawer .thumbstrip::-webkit-scrollbar-thumb {
  background: #444;
  border-radius: 3px;
}
.detail-drawer .thumbstrip img {
  flex: 0 0 auto;
  height: 56px;
  width: auto;
  border-radius: 4px;
  cursor: pointer;
  opacity: 0.45;
  border: 2px solid transparent;
  object-fit: cover;
  transition: opacity 0.15s, border-color 0.15s;
}
.detail-drawer .thumbstrip img:hover { opacity: 0.8; }
.detail-drawer .thumbstrip img.active {
  opacity: 1;
  border-color: #f5c518;
}

</style>
</head>
<body>
<h1>Movie Wall</h1>
<div class="poster-wall" id="wall"></div>
<script>
function stars(r) {
  r = Math.max(0, Math.min(10, Math.round(Number(r) || 0)));
  let out = '';
  for (let i = 1; i <= 10; i++) {
    out += `<span class="star ${i <= r ? 'on' : 'off'}" data-value="${i}">★</span>`;
  }
  return out;
}

function rateClass(r) {
  if (r <= 0) return 'none';
  if (r <= 4) return 'low';
  if (r <= 7) return 'mid';
  return 'high';
}

function ratingHtml(r) {
  const v = Math.max(0, Math.min(10, Math.round(Number(r) || 0)));
  return `<span class="stars">${stars(v)}</span>`
       + `<span class="score ${rateClass(v)}">${v}</span>`;
}

function msToHMS(ms) {
  const total = Math.max(0, Math.floor(Number(ms) || 0));
  const s = Math.floor(total / 1000);
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  const pad = n => String(n).padStart(2, '0');
  return `${pad(h)}:${pad(m)}:${pad(sec)}`;
}

function formatFileSizeGMKB(total) {
  const n = Number(total) || 0;
  const G = 1024 * 1024 * 1024;
  const M = 1024 * 1024;
  const K = 1024;

  const g = Math.floor(n / G);
  const remG = n % G;
  const m = Math.floor(remG / M);
  const remM = remG % M;
  const k = Math.floor(remM / K);
  const b = remM % K;

  return `${g}'${m}'${k}'${b}`;
}

async function load() {
  const movies = await (await fetch('/api/movies')).json();
  const wall = document.getElementById('wall');
  wall.innerHTML = movies.map(m => `
    <div class="poster" data-id="${m.id}" data-rate="${m.rate}">
      <img src="${m.thumbUrl}" loading="lazy" alt="">
      <div class="meta">
        <div class="duration" title="Video Duration">${msToHMS(m.duration)}</div>
        <div class="size" title="Video Size">${formatFileSizeGMKB(m.vidSize)}</div>
        <div class="rating">${ratingHtml(m.rate)}</div>
      </div>
      <div class="name" title="${m.name}">${m.name}</div>
    </div>
  `).join('');
}
load();

// 点击标题/详情页标题 播放
async function playVideo(id) {
  try {
    const res = await fetch('/api/openvideo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id }),
    });
    const data = await res.json();
    if (!data.ok) console.warn('[video] failed:', data.error);
  } catch (err) {
    console.error('[video] request failed:', err);
  }
}

// 事件委托：点击任意星星
document.getElementById('wall').addEventListener('click', async (e) => {
  // 1. 星星：立即改评分
  const star = e.target.closest('.star');
  if (star) {
    const poster = star.closest('.poster');
    if (!poster) return;
    const id = Number(poster.dataset.id);
    const newRate = Number(star.dataset.value);
    const oldRate = Number(poster.dataset.rate);
    if (newRate === oldRate) return;
    try {
      const res = await fetch('/api/rate', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id, rate: newRate }),
      });
      const data = await res.json();
      if (data.ok) {
        poster.dataset.rate = String(newRate);
        poster.querySelector('.rating').innerHTML = ratingHtml(newRate);
      } else {
        console.warn('[rate] failed:', data.error);
      }
    } catch (err) {
      console.error('[rate] request failed:', err);
    }
    return;
  }

  // 2. 卡片标题：播放视频
  const nameEl = e.target.closest('.poster .name');
  if (nameEl) {
    const poster = nameEl.closest('.poster');
    if (!poster) return;
    playVideo(Number(poster.dataset.id));
    return;
  }

  // 3. 海报图片：立即打开详情
  const img = e.target.closest('.poster img');
  if (!img) return;
  const poster = img.closest('.poster');
  openDetailDrawer(Number(poster.dataset.id));
});

// ---------- 详情抽屉 ----------
const detailState = {
  id: -1,
  imgs: [],
  idx: 0,
};

const detailBackdrop = document.createElement('div');
detailBackdrop.className = 'detail-backdrop';
document.body.appendChild(detailBackdrop);

const detailDrawer = document.createElement('div');
detailDrawer.className = 'detail-drawer';
detailDrawer.innerHTML = `
  <div class="handle"></div>
  <div class="head">
    <span class="title"></span>
    <button class="close-btn" title="Close (Esc)">×</button>
  </div>
  <div class="sub"></div>
  <div class="gallery">
    <div class="index-bar">
      <button class="nav-btn nav-prev" title="Previous (←)">‹</button>
      <span class="index-text">0 / 0</span>
      <button class="nav-btn nav-next" title="Next (→)">›</button>
    </div>
    <div class="hero"><img alt=""></div>
    <div class="thumbstrip"></div>
  </div>`;
document.body.appendChild(detailDrawer);

function renderDetailView() {
  const total = detailState.imgs.length;
  if (total === 0) return;
  const idx = detailState.idx;

  detailDrawer.querySelector('.index-text').textContent = `${idx + 1} / ${total}`;
  detailDrawer.querySelector('.hero img').src = detailState.imgs[idx];

  detailDrawer.querySelectorAll('.thumbstrip img').forEach((t, i) => {
    t.classList.toggle('active', i === idx);
  });

  const active = detailDrawer.querySelector('.thumbstrip img.active');
  if (active) {
    active.scrollIntoView({ behavior: 'smooth', block: 'nearest', inline: 'center' });
  }

  detailDrawer.querySelector('.nav-prev').disabled = (idx <= 0);
  detailDrawer.querySelector('.nav-next').disabled = (idx >= total - 1);
}

function setDetailIndex(newIdx) {
  if (newIdx < 0 || newIdx >= detailState.imgs.length) return;  // 边界不响应
  detailState.idx = newIdx;
  renderDetailView();
}

function onDetailKey(e) {
  if (e.key === 'Escape') { closeDetail(); return; }

  if (e.key === 'ArrowLeft') {
    if (detailState.idx <= 0) return;               // 首图不响应左键
    e.preventDefault();
    setDetailIndex(detailState.idx - 1);
  } else if (e.key === 'ArrowRight') {
    if (detailState.idx >= detailState.imgs.length - 1) return;  // 尾图不响应右键
    e.preventDefault();
    setDetailIndex(detailState.idx + 1);
  }
}

function closeDetail() {
  detailDrawer.classList.remove('open');
  detailBackdrop.classList.remove('open');
  document.removeEventListener('keydown', onDetailKey);

  // 释放大图引用，避免闭包持有
  detailState.id   = -1;
  detailState.imgs = [];
  detailState.idx  = 0;
}

async function openDetailDrawer(id) {
  // 先打开空抽屉，视觉上立即响应
  detailDrawer.querySelector('.title').textContent = 'Loading...';
  detailDrawer.querySelector('.sub').textContent = '';
  detailDrawer.querySelector('.index-text').textContent = '0 / 0';
  detailDrawer.querySelector('.hero img').removeAttribute('src');
  detailDrawer.querySelector('.thumbstrip').innerHTML = '';
  detailDrawer.classList.add('open');
  detailBackdrop.classList.add('open');
  document.addEventListener('keydown', onDetailKey);

  try {
    const res = await fetch(`/api/detail/${id}`);
    const d = await res.json();
    if (d.ok === false) { closeDetail(); return; }

    detailDrawer.querySelector('.title').textContent = d.name || '(untitled)';
    detailDrawer.querySelector('.sub').innerHTML =
        `<span>${msToHMS(d.duration)}</span>`
      + `<span>${formatFileSizeGMKB(d.vidSize)}</span>`
      + `<span>${d.rate} / 10</span>`;

    detailState.id   = id;
    detailState.imgs = d.imgs || [];
    detailState.idx  = 0;

    const thumbstrip = detailDrawer.querySelector('.thumbstrip');
    thumbstrip.innerHTML = detailState.imgs
      .map((src, i) => `<img src="${src}" data-idx="${i}" alt="">`)
      .join('');

    thumbstrip.querySelectorAll('img').forEach(img => {
      img.addEventListener('click', () => {
        setDetailIndex(Number(img.dataset.idx));
      });
    });

    // 前后按钮也接上
    detailDrawer.querySelector('.nav-prev').onclick = () => setDetailIndex(detailState.idx - 1);
    detailDrawer.querySelector('.nav-next').onclick = () => setDetailIndex(detailState.idx + 1);

    if (detailState.imgs.length === 0) {
      detailDrawer.querySelector('.index-text').textContent = '0 / 0';
    } else {
      renderDetailView();
    }
  } catch (err) {
    console.error('[detail] failed:', err);
    closeDetail();
  }
}

detailDrawer.querySelector('.head .title').addEventListener('click', () => {
  if (detailState.id >= 0) playVideo(detailState.id);
});
detailBackdrop.addEventListener('click', closeDetail);
detailDrawer.querySelector('.close-btn').addEventListener('click', closeDetail);

</script>
</body>
</html>)HTML";
}