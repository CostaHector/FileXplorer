#include "SqlRecordTestHelper.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "FdBasedDb.h"
#include <QSqlField>
#include <QVariant>

namespace SqlRecordTestHelper {

QSqlRecord GetACastRecordLine(const QString& castName, const QString& ori, const QString& imgs, const QString& vids) {
  using namespace PERFORMER_DB_HEADER_KEY;
  QSqlRecord rec;
  rec.append(QSqlField(ENUM_2_STR(Name)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Rate)  , QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(AKA)   , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Tags)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Ori)   , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Vids)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Imgs)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Detail), QVariant::String));

  rec.setValue(Name, castName);
  rec.setValue(Rate, 9);
  rec.setValue(AKA, "Thor");
  rec.setValue(Tags, "Man");
  rec.setValue(Ori, ori);
  rec.setValue(Vids, vids);
  rec.setValue(Imgs, imgs);
  rec.setValue(Detail, "The Australian actor first appeared as Thor in 2011 and has since reprised the role in numerous MCU films");
  return rec;
}

QSqlRecord GetAMovieRecordUsedInBrowser(const QString& prePathLeft, const QString& prePathRight, const QString& name, qint64 sz) {
  using namespace MOVIE_TABLE;
  QSqlRecord rec;
  rec.append(QSqlField(ENUM_2_STR(PrePathLeft)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(PrePathRight)  , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Name)   , QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Size)  , QVariant::LongLong));
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::PrePathLeft, prePathLeft);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::PrePathRight, prePathRight);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::Name, name);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::Size, sz);
  return rec;
}

}
