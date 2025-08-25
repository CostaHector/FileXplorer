#ifndef PERFORMERJSONFILEHELPER_H
#define PERFORMERJSONFILEHELPER_H

#include <QSqlRecord>
#include <QString>
#include <QVariantHash>

class PerformerJsonFileHelper {
 public:
  static QString PsonPath(const QString& imageHostPath, const QVariantHash& pson);

  static QString PsonPath(const QString& imageHostPath, const QString& ori, const QString& castName) {
    return imageHostPath + '/' + ori + '/' + castName + '/' + castName + ".pson";
  }

  static QVariantHash PerformerJsonJoiner(const QSqlRecord& record);

  static QVariantHash PerformerJsonJoiner(const QString& name,
                                          const int rate = DEFAULT_RATE,
                                          const QString& aka = "",
                                          const QString& tags = "",
                                          const QString& ori = DEFAULT_ORIENTATION,
                                          const QString& vids = "",
                                          const QString& imgs = "",
                                          const QString& detail = "");
  static constexpr int DEFAULT_RATE{0};
  static const QString DEFAULT_ORIENTATION;
};

#endif  // PERFORMERJSONFILEHELPER_H
