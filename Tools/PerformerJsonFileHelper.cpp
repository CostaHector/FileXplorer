#include "PerformerJsonFileHelper.h"

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

  return QString{"REPLACE INTO `%1` (%2) VALUES(%3);"}.arg(tableName).arg(updateKey.join(',')).arg(updateValue.join(','));
}
