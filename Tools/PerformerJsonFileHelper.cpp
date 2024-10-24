#include "PerformerJsonFileHelper.h"

const QRegularExpression PerformerJsonFileHelper::IMG_VID_SEP_COMP("\\||\r\n|\n");
constexpr char PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR;

QString PerformerJsonFileHelper::CreatePerformerTableSQL(const QString& tableName) {
  return QString(
             "CREATE TABLE IF NOT EXISTS `%1`("
             "   `%2` TEXT NOT NULL,"
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

QVariantHash PerformerJsonFileHelper::PerformerJsonJoiner(const QSqlRecord& record) {
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

QString PerformerJsonFileHelper::PerformerInsertSQL(const QString& tableName,
                                                    const QString& name,
                                                    const int rate,
                                                    const QString& aka,
                                                    const QString& tags,
                                                    const QString& ori,
                                                    const QString& vids,
                                                    const QString& imgs,
                                                    QString detail) {
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
    updateValue << '\"' + detail.replace('"', "\"\"") + '\"';
  }

  return QString("REPLACE INTO `%1` (%2) VALUES(%3);").arg(tableName).arg(updateKey.join(',')).arg(updateValue.join(','));
}
