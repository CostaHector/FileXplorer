#include "PerformerJsonFileHelper.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include <QSqlField>

constexpr int PerformerJsonFileHelper::DEFAULT_RATE;
const QString PerformerJsonFileHelper::DEFAULT_ORIENTATION{"gay"};

QString PerformerJsonFileHelper::PsonPath(const QString& imageHostPath, const QVariantHash& pson) {
  using namespace PERFORMER_DB_HEADER_KEY;
  return PsonPath(imageHostPath, pson[ENUM_2_STR(Ori)].toString(), pson[ENUM_2_STR(Name)].toString());
}

QVariantHash PerformerJsonFileHelper::PerformerJsonJoiner(const QSqlRecord& record) {
  using namespace PERFORMER_DB_HEADER_KEY;
  const QString& name =   record.field(Name).value().toString();
  const int rate =        record.field(Rate).value().toInt();
  const QString& aka =    record.field(AKA).value().toString();
  const QString& tags =   record.field(Tags).value().toString();
  const QString& ori =    record.field(Ori).value().toString();
  const QString& vids =   record.field(Vids).value().toString();
  const QString& imgs =   record.field(Imgs).value().toString();
  const QString& detail = record.field(Detail).value().toString();
  return PerformerJsonFileHelper::PerformerJsonJoiner(name, rate, aka, tags, ori, vids, imgs, detail);
}

QVariantHash PerformerJsonFileHelper::PerformerJsonJoiner(//
    const QString& name,
    const int rate,
    const QString& aka,
    const QString& tags,
    const QString& ori,
    const QString& vids,
    const QString& imgs,
    const QString& detail) {
  using namespace PERFORMER_DB_HEADER_KEY;
  return {
      {ENUM_2_STR(Name), name}, //
      {ENUM_2_STR(Rate), rate}, //
      {ENUM_2_STR(AKA), aka}, //
      {ENUM_2_STR(Tags), tags}, //
      {ENUM_2_STR(Ori), ori}, //
      {ENUM_2_STR(Vids), vids}, //
      {ENUM_2_STR(Imgs), imgs}, //
      {ENUM_2_STR(Detail), detail} //
  };
}
