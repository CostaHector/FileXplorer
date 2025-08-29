#include "CastPsonFileHelper.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include <QSqlField>

namespace CastPsonFileHelper {

QString PsonPath(const QString& imageHostPath, const QVariantHash& pson) {
  using namespace PERFORMER_DB_HEADER_KEY;
  return PsonPath(imageHostPath, pson[ENUM_2_STR(Ori)].toString(), pson[ENUM_2_STR(Name)].toString());
}

QVariantHash PerformerJsonJoiner(const QSqlRecord& record) {
  using namespace PERFORMER_DB_HEADER_KEY;
  const QString& name   = record.field(Name).value().toString();
  const int rate        = record.field(Rate).value().toInt();
  const QString& aka    = record.field(AKA).value().toString();
  const QString& tags   = record.field(Tags).value().toString();
  const QString& ori    = record.field(Ori).value().toString();
  const QString& vids   = record.field(Vids).value().toString();
  const QString& imgs   = record.field(Imgs).value().toString();
  const QString& detail = record.field(Detail).value().toString();
  return PerformerJsonJoiner(name, rate, aka, tags, ori, vids, imgs, detail);
}

QVariantHash PerformerJsonJoiner(const QString& name,
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

QByteArray CastValues2PsonStr(const QString& name,
                              const int rate,
                              const QString& aka,
                              const QString& tags,
                              const QString& ori,
                              const QString& vids,
                              const QString& imgs,
                              const QString& detail) {
  const QString rateS{QString::number(rate)};
  using namespace PERFORMER_DB_HEADER_KEY;
  QString psonString;
  psonString.reserve(200);
  psonString+="{";
  psonString+=R"(")"; psonString+=ENUM_2_STR(Name)  ; psonString+=R"(":")"; psonString+= name  ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Rate)  ; psonString+=R"(":")"; psonString+= rateS ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(AKA)   ; psonString+=R"(":")"; psonString+= aka   ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Tags)  ; psonString+=R"(":")"; psonString+= tags  ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Ori)   ; psonString+=R"(":")"; psonString+= ori   ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Vids)  ; psonString+=R"(":")"; psonString+= vids  ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Imgs)  ; psonString+=R"(":")"; psonString+= imgs  ;psonString+=R"(",)"; //
  psonString+=R"(")"; psonString+=ENUM_2_STR(Detail); psonString+=R"(":")"; psonString+= detail;psonString+=R"(" )"; //
  psonString += "}";
  return psonString.toUtf8();
}

}
