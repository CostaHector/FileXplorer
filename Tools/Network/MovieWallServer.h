#ifndef MOVIEWALLSERVER_H
#define MOVIEWALLSERVER_H

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QCache>
#include <QMutex>
#include <thread>

#include "FolderNxtAndLastIterator.h"
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

  explicit MovieWallServer(QObject* parent = nullptr, bool bIncludeSubdirectory = false);
  ~MovieWallServer() override;

  bool isIncludeSubdirectory() const { return m_bSubdirectories; }
  bool setRootPath(const QString& rootPath);
  bool start(quint16 port);
  void stop();
  QString urlAddress() const { return QString::asprintf("http://%s:%d", DEFAULT_MOVIE_WALL_HOST, port()); }
  quint16 port() const { return m_port; }

private:
  bool onIteratorToAnotherFolderCore(bool isNext);
  bool onIncludeSubdirectoryChanged(bool bIncludeSubdirectory);

  // 在 MovieWallServer 所属线程（主线程）执行 fn，返回其返回值
  bool invokeOnMainThread(std::function<bool()> fn);

  std::string buildMoviesJson() const;
  QImage getThumbnail(const QString& thumbImgFullPath, const int eleId) const;
  // 内部持有 m_sceneMutex，调用方不得已持有 m_sceneMutex
  QString resolveImage(int id, int imgIdx) const;
  QString resolveThumbnailImage(int id) const;
  static bool GetQImage(const QString& path, QImage& img, bool enablePlaceholderImage, bool isDetailImage);
  QJsonObject GetQJsonObject(int id, bool bIsDetail) const;

  bool PostApiThumbIdCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiRateCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiOpenVideoCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiDetailCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiDetailImgCallback(const httplib::Request& req, httplib::Response& res);
  bool PostApiSetSubdirectories(const httplib::Request& req, httplib::Response& res);
  bool PostApiSetRootPath(const httplib::Request& req, httplib::Response& res);
  bool PostApiNavigateFolder(const httplib::Request& req, httplib::Response& res);

  static QByteArray indexHtml();

  QString mRootPath;
  bool m_bSubdirectories;
  FolderNxtAndLastIterator mFolderIterator;
  SceneInfoList m_sceneElements;
  mutable QMutex m_sceneMutex; // mSceneElements 中 rate（读+写）

  quint16 m_port = 0;
  std::thread m_thread;
  void* m_server = nullptr;
  mutable QCache<QString, QImage> m_thumbCache;
  mutable QMutex m_cacheMutex;
};

#endif // MOVIEWALLSERVER_H
