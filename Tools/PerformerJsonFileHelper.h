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

  static QString CreatePerformerTableSQL(const QString& tableName) {
    return QString(
               "CREATE TABLE IF NOT EXISTS `%1`("
               "   `%2` TEXT UNIQUE NOT NULL,"
               "   `%3` INT DEFAULT %4,"
               "   `%5` TEXT DEFAULT \"\","
               "   `%6` TEXT DEFAULT \"\","
               "   `%7` TEXT DEFAULT \"%8\","
               "   `%9` TEXT DEFAULT \"\","
               "   `%10` TEXT DEFAULT \"\","
               "   `%11` TEXT DEFAULT \"\","
               "    PRIMARY KEY (%2)"
               "    );")
        .arg(tableName)
        .arg(PERFORMER_DB_HEADER_KEY::Name)
        .arg(PERFORMER_DB_HEADER_KEY::Rate)
        .arg(PERFORMER_DB_HEADER_KEY::DEFAULT_RATE)
        .arg(PERFORMER_DB_HEADER_KEY::AKA)
        .arg(PERFORMER_DB_HEADER_KEY::Tags)
        .arg(PERFORMER_DB_HEADER_KEY::Orientation)
        .arg(PERFORMER_DB_HEADER_KEY::DEFAULT_ORIENTATION)
        .arg(PERFORMER_DB_HEADER_KEY::Vids)
        .arg(PERFORMER_DB_HEADER_KEY::Imgs)
        .arg(PERFORMER_DB_HEADER_KEY::Detail);
  }

  static QVariantHash PerformerJsonJoiner(const QSqlRecord& record) {
    const QString& name = record.field(PERFORMER_DB_HEADER_KEY::Name_INDEX).value().toString();
    const int rates = record.field(PERFORMER_DB_HEADER_KEY::Rate_INDEX).value().toInt();
    const QString& akas = record.field(PERFORMER_DB_HEADER_KEY::AKA_INDEX).value().toString();
    const QString& tags = record.field(PERFORMER_DB_HEADER_KEY::Tags_INDEX).value().toString();
    const QString& oris = record.field(PERFORMER_DB_HEADER_KEY::Orientation_INDEX).value().toString();
    const QString& vids = record.field(PERFORMER_DB_HEADER_KEY::Vids_INDEX).value().toString();
    const QString& imgs = record.field(PERFORMER_DB_HEADER_KEY::Imgs_INDEX).value().toString();
    const QString& details = record.field(PERFORMER_DB_HEADER_KEY::Detail_INDEX).value().toString();
    return PerformerJsonFileHelper::PerformerJsonJoiner(name, rates, akas, tags, oris, vids, imgs, details);
  }

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
                                    const QString& detail = "") {
    if (name.isEmpty()) {
      qDebug("main key cannot be null.");
      return "";
    }

    QStringList updateKey{PERFORMER_DB_HEADER_KEY::Name};
    QStringList updateValue;
    updateValue << "\"" + name + "\"";
    if (rate != PERFORMER_DB_HEADER_KEY::DEFAULT_RATE) {
      updateKey << PERFORMER_DB_HEADER_KEY::Rate;
      updateValue << QString::number(rate);
    }
    if (not aka.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::AKA;
      updateValue << "\"" + aka + "\"";
    }
    if (not tags.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::Tags;
      updateValue << "\"" + tags + "\"";
    }
    if (not ori.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::Orientation;
      updateValue << "\"" + ori + "\"";
    }
    if (not vids.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::Vids;
      updateValue << "\"" + vids + "\"";
    }
    if (not imgs.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::Imgs;
      updateValue << "\"" + imgs + "\"";
    }
    if (not detail.isEmpty()) {
      updateKey << PERFORMER_DB_HEADER_KEY::Detail;
      updateValue << "\"" + detail + "\"";
    }

    return QString("REPLACE INTO `%1` (%2) VALUES(%3);").arg(tableName).arg(updateKey.join(',')).arg(updateValue.join(','));
  }
};

#endif  // PERFORMERJSONFILEHELPER_H
