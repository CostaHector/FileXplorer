#ifndef CASTPSONFILEHELPER_H
#define CASTPSONFILEHELPER_H

#include <QSqlRecord>
#include <QString>
#include <QVariantHash>

namespace CastPsonFileHelper {
constexpr int DEFAULT_RATE{0};
const QString DEFAULT_ORIENTATION{"gay"};

QString PsonPath(const QString& imageHostPath, const QVariantHash& pson);

inline QString PsonPath(const QString& imageHostPath, const QString& ori, const QString& castName) {
  return imageHostPath + '/' + ori + '/' + castName + '/' + castName + ".pson";
}

QVariantHash PerformerJsonJoiner(const QSqlRecord& record);

QVariantHash PerformerJsonJoiner(const QString& name,
                                 const int rate = DEFAULT_RATE,
                                 const QString& aka = "",
                                 const QString& tags = "",
                                 const QString& ori = DEFAULT_ORIENTATION,
                                 const QString& vids = "",
                                 const QString& imgs = "",
                                 const QString& detail = "",
                                 const int heightInt = -1,
                                 const int sizeInt = -1,
                                 const QString& birthStr = "");
QByteArray CastValues2PsonStr(const QString& name,
                              const int rate = DEFAULT_RATE,
                              const QString& aka = "",
                              const QString& tags = "",
                              const QString& ori = DEFAULT_ORIENTATION,
                              const QString& vids = "",
                              const QString& imgs = "",
                              const QString& detail = "",
                              const int heightInt = -1,
                              const int sizeInt = -1,
                              const QString& birthStr = "");
}

#endif  // CASTPSONFILEHELPER_H
