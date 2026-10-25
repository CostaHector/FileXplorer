#ifndef MOVIEWALLSERVER_H
#define MOVIEWALLSERVER_H

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QCache>
#include <QMutex>
#include <thread>

#include "SceneInfo.h"

namespace httplib{
class Request;
class Response;
};

class MovieWallServer : public QObject {
  Q_OBJECT
public:
  static constexpr const char* DEFAULT_MOVIE_WALL_HOST = "127.0.0.1";
  static constexpr int DEFAULT_MOVIE_WALL_PORT = 8766;

  explicit MovieWallServer(QObject* parent = nullptr);
  ~MovieWallServer() override;
  void setRootPath(const QString rootPath, SceneInfoList* pScenesList);
  bool start(quint16 port);
  void stop();
  QString urlAddress() const { return QString::asprintf("http://%s:%d?v=%lld", DEFAULT_MOVIE_WALL_HOST, port(), dataVersion()); }
  quint16 port() const { return m_port; }

private:
  std::string buildMoviesJson() const;
  QImage getThumbnail(int id) const;
  QString resolveVideo(int id) const;
  QString resolveImage(int id, int imgIdx) const;
  static bool GetQImage(const QString& path, QImage& img, bool enablePlaceholderImage, bool isDetailImage);
  QJsonObject GetQJsonObject(int id, bool bIsDetail) const;

  bool PostApiThumbIdCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiRateCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiOpenVideoCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiDetailCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiDetailImgCallback(const httplib::Request& req, httplib::Response& res);
  static const char* indexHtml();

  QString mRootPath;
  SceneInfoList* _mPSceneElements{nullptr};
  mutable QMutex m_sceneMutex; // mSceneElements 中 rate（读+写）

  quint16 m_port = 0;
  std::thread m_thread;
  void* m_server = nullptr;
  mutable QCache<int, QImage> m_thumbCache;
  mutable QMutex m_cacheMutex;

  quint64 dataVersion() const { return m_dataVersion.load(std::memory_order_acquire); }
  std::atomic<quint64> m_dataVersion{0};
};

#endif // MOVIEWALLSERVER_H
