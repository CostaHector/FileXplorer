#ifndef VIDEODURATIONGETTER_H
#define VIDEODURATIONGETTER_H
#include <QString>
#include <QList>

class VideoDurationGetter {
 public:
  static double ReadAVideo(const QString& vidPath);
  static QList<double> ReadVideos(const QStringList& vidsPath);
};

#endif // VIDEODURATIONGETTER_H
