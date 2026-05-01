#ifndef QMEDIAINFO_H
#define QMEDIAINFO_H

#include <memory>
#include <QStringList>

namespace MediaInfoDLL {
class MediaInfo;
}

class QMediaInfo {
public:
  static QMediaInfo& GetInst();
  bool Open(const QString& filename);
  int DurationLengthQuick(const QString& fileAbsPath);
  QList<int> batchVidsDurationLength(const QStringList& vidsAbsPath);
  explicit operator bool() const { return isLoadDllOk(); }

private:
  explicit QMediaInfo();
  QMediaInfo(const QMediaInfo& rhs) = delete;

  bool isLoadDllOk() const { return m_bLoadDllResult; }
  std::unique_ptr<MediaInfoDLL::MediaInfo> m_mediaInfo;
  const bool m_bLoadDllResult;
};

#endif // QMEDIAINFO_H
