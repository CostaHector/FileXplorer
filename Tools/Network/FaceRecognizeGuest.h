#ifndef FACE_RECOGNIZE_GUEST_H
#define FACE_RECOGNIZE_GUEST_H

#include <QJsonArray>
#include <QNetworkAccessManager>
class QPixmap;
class QLabel;

class FaceRecognizeGuest {
public:
  static FaceRecognizeGuest& GetInst();
  FaceRecognizeGuest() = default;
  bool PostRecognize(const QPixmap& pixmap, QLabel* pResultLabel, int topK = 1);

  QString GetMatchResultStr(const QJsonArray& names, const QJsonArray& scores);
private:
  QNetworkAccessManager mManager;
};

#endif