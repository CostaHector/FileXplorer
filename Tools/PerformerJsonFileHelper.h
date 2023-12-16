#ifndef PERFORMERJSONFILEHELPER_H
#define PERFORMERJSONFILEHELPER_H

#include <QRegExp>
#include <QSqlField>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QVariantHash>

namespace PERFORMER_DB_HEADER_KEY {
const QString Name = "Name";
const QString Rate = "Rate";
const QString AKA = "AKA";
const QString Tags = "Tags";        // "Daddy", "BBC", "White cock", "Mature", "bodybuilder", "atheletic", "beard", "asian", "leather"
const QString Orientation = "Ori";  // ["bi", "gay", "hetero"]
const QString Vids = "Vids";
const QString Imgs = "Imgs";
const QString Detail = "Detail";
const QStringList DB_HEADER{Name, Rate, AKA, Tags, Orientation, Vids, Imgs, Detail};

const int DEFAULT_RATE = 0;
const QString DEFAULT_ORIENTATION = "gay";

const int Name_INDEX = DB_HEADER.indexOf(Name);
const int Rate_INDEX = DB_HEADER.indexOf(Rate);
const int AKA_INDEX = DB_HEADER.indexOf(AKA);
const int Tags_INDEX = DB_HEADER.indexOf(Tags);
const int Orientation_INDEX = DB_HEADER.indexOf(Orientation);
const int Vids_INDEX = DB_HEADER.indexOf(Vids);
const int Imgs_INDEX = DB_HEADER.indexOf(Imgs);
const int Detail_INDEX = DB_HEADER.indexOf(Detail);
}  // namespace PERFORMER_DB_HEADER_KEY

class PerformerJsonFileHelper {
 public:
  static const QRegExp IMG_VID_SEP_COMP;

  static QString GetFirstPerformerImgPath(const QString& m_imageHostPath, const QString& ori, const QString& performerName, const QString& imgs) {
    const int i = imgs.indexOf(IMG_VID_SEP_COMP);
    const QString& firstImg = (i == -1) ? imgs : imgs.left(i);
    return (firstImg.isEmpty()) ? "" : m_imageHostPath + '/' + ori + '/' + performerName + '/' + firstImg;
  }

  static QString PJsonPath(const QString& m_imageHostPath, const QVariantHash& pJson) {
    return PJsonPath(m_imageHostPath, pJson[PERFORMER_DB_HEADER_KEY::Orientation].toString(), pJson[PERFORMER_DB_HEADER_KEY::Name].toString());
  }

  static QString PJsonPath(const QString& m_imageHostPath, const QString& ori, const QString& performerName) {
    return m_imageHostPath + '/' + ori + '/' + performerName + '/' + performerName + ".pjson";
  }

  static QString CreatePerformerTableSQL(const QString& tableName);

  static QVariantHash PerformerJsonJoiner(const QSqlRecord& record);

  static QVariantHash PerformerJsonJoiner(const QString& name,
                                          const int rate = PERFORMER_DB_HEADER_KEY::DEFAULT_RATE,
                                          const QString& aka = "",
                                          const QString& tags = "",
                                          const QString& ori = PERFORMER_DB_HEADER_KEY::DEFAULT_ORIENTATION,
                                          const QString& vids = "",
                                          const QString& imgs = "",
                                          const QString& detail = "") {
    return {{PERFORMER_DB_HEADER_KEY::Name, name}, {PERFORMER_DB_HEADER_KEY::Rate, rate},       {PERFORMER_DB_HEADER_KEY::AKA, aka},
            {PERFORMER_DB_HEADER_KEY::Tags, tags}, {PERFORMER_DB_HEADER_KEY::Orientation, ori}, {PERFORMER_DB_HEADER_KEY::Vids, vids},
            {PERFORMER_DB_HEADER_KEY::Imgs, imgs}, {PERFORMER_DB_HEADER_KEY::Detail, detail}};
  }

  static QString PerformerInsertSQL(const QString& tableName, const QVariantHash& pJson) {
    return PerformerInsertSQL(tableName, pJson[PERFORMER_DB_HEADER_KEY::Name].toString(), pJson[PERFORMER_DB_HEADER_KEY::Rate].toInt(),
                              pJson[PERFORMER_DB_HEADER_KEY::AKA].toString(), pJson[PERFORMER_DB_HEADER_KEY::Tags].toString(),
                              pJson[PERFORMER_DB_HEADER_KEY::Orientation].toString(), pJson[PERFORMER_DB_HEADER_KEY::Vids].toString(),
                              pJson[PERFORMER_DB_HEADER_KEY::Imgs].toString(), pJson[PERFORMER_DB_HEADER_KEY::Detail].toString());
  }

  static QString PerformerInsertSQL(const QString& tableName,
                                    const QString& name,
                                    const int rate = PERFORMER_DB_HEADER_KEY::DEFAULT_RATE,
                                    const QString& aka = "",
                                    const QString& tags = "",
                                    const QString& ori = PERFORMER_DB_HEADER_KEY::DEFAULT_ORIENTATION,
                                    const QString& vids = "",
                                    const QString& imgs = "",
                                    QString detail = "");

  static constexpr char PERFS_VIDS_IMGS_SPLIT_CHAR = '\n';
};

#endif  // PERFORMERJSONFILEHELPER_H
