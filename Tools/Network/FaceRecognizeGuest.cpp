#include "FaceRecognizeGuest.h"
#include "Logger.h"

#include <QLabel>
#include <QPixmap>
#include <QBuffer>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

FaceRecognizeGuest &FaceRecognizeGuest::GetInst() {
  static FaceRecognizeGuest inst;
  return inst;
}

QByteArray EncodeImage(const QPixmap &pixmap, const QString& noDotSuffix="PNG") {
  QByteArray bytes;
  QBuffer buffer(&bytes);
  buffer.open(QIODevice::WriteOnly);

  // QPixmap -> QImage，转成 RGB888 去掉 alpha，避免透明区域变黑
  QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
  if (!image.save(&buffer, noDotSuffix.toUtf8())) {
    return {};
  }
  return bytes;
}

bool FaceRecognizeGuest::PostRecognize(const QPixmap &pixmap, QLabel* pResultLabel, int topK) {
  if (pixmap.isNull()) {
    LOG_D("QPixmap is empty");
    if (pResultLabel != nullptr) pResultLabel->setText("Error. QPixmap is empty");
    return false;
  }
  // QPixmap -> PNG 字节
  const QByteArray imgBA{EncodeImage(pixmap)};
  if (imgBA.isEmpty()) {
    LOG_D("Image Code failed");
    if (pResultLabel != nullptr) pResultLabel->setText("Error. Image Code failed");
    return false;
  }

  QUrl url("http://127.0.0.1:8765/recognize");
  {
    QUrlQuery query;
    query.addQueryItem("topK", QString::number(topK > 0 ? topK : 1));
    url.setQuery(query);
  }

  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

  LOG_D("Requesting...");
  if (pResultLabel != nullptr) pResultLabel->setText("Requesting...");
  QNetworkReply *reply = mManager.post(req, imgBA);

  QObject::connect(reply, &QNetworkReply::finished, [reply, pResultLabel, this]() {
    const QByteArray data = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const bool bIsObj{doc.isObject()};
    const QNetworkReply::NetworkError err = reply->error();
    if (err != QNetworkReply::NoError) {
      if (bIsObj && doc.object().contains("error")) {
        if (pResultLabel != nullptr) pResultLabel->setText("Error find in server reply");
        LOG_W("Error find in server reply: %s", qPrintable(doc.object().value("error").toString()));
      } else {
        if (pResultLabel != nullptr) pResultLabel->setText("Network error");
        LOG_W("Network error: [code:%d]%s. body:[%s]", err, qPrintable(reply->errorString()), data.constData());
      }
      return;
    }

    if (!bIsObj) {
      if (pResultLabel != nullptr) pResultLabel->setText("Error parse response. See detail in log");
      LOG_W("Cannot parse response: %s", data.constData());
      return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("error")) {
      if (pResultLabel != nullptr) pResultLabel->setText("Error find in server reply. See detail in log");
      LOG_W("Host return error: %s", qPrintable(obj.value("error").toString()));
      return;
    }

    const QJsonArray faceNamesList = obj.value("faceNamesList").toArray();
    const QJsonArray similarityScoresList  = obj.value("similarityScoresList").toArray();

    QString matchResult = GetMatchResultStr(faceNamesList, similarityScoresList);
    LOG_W("Recognize Result:%s", qPrintable(matchResult));
    if (pResultLabel != nullptr) pResultLabel->setText(matchResult);
  });
  return true;
}

QString FaceRecognizeGuest::GetMatchResultStr(const QJsonArray &names, const QJsonArray &scores) {
  if (names.isEmpty() || names.size() != scores.size()) {
    return QString::asprintf("No Match find at all [%d, %d]", names.size(), scores.size());
  }
  QString msg;
  msg += "Rate Name\n";
  const int n = qMin(names.size(), scores.size());
  for (int i = 0; i < n; ++i) {
    msg += QString::asprintf("%.2f %s\n", scores[i].toDouble(0), qPrintable(names[i].toString()));
  }
  return msg;
}