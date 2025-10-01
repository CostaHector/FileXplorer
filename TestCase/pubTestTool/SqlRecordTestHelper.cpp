#include "SqlRecordTestHelper.h"
#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "FdBasedDb.h"
#include <QSqlField>
#include <QVariant>
#include <QDebug>

namespace SqlRecordTestHelper {

QSqlRecord GetACastRecordLine(const QString& castName, const QString& ori, const QString& imgs, const QString& vids) {
  using namespace PERFORMER_DB_HEADER_KEY;
  QSqlRecord rec;
  rec.append(QSqlField(ENUM_2_STR(Name), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Rate), QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(AKA), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Tags), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Ori), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Vids), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Imgs), QVariant::String));
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

bool CheckRecordIfEqual(const QSqlRecord& actualRec,
                        const QString& name,
                        const int rate,
                        const QString& aka,
                        const QString& tags,
                        const QString& ori,
                        const QString& vids,
                        const QString& imgs,
                        const QString& detail,
                        bool fullMatch) {
  using namespace PERFORMER_DB_HEADER_KEY;
  const QVariantHash expectedValues = CastPsonFileHelper::PerformerJsonJoiner(name, rate, aka, tags, ori, vids, imgs, detail);

  bool allMatch = true;
  const QStringList& expectKeys = expectedValues.keys();
  for (const auto& key : expectKeys) {
    if (!actualRec.contains(key)) {
      qDebug() << "Field:" << key << " not found";  //
      allMatch = false;
      continue;
    }
    QVariant actualValue = actualRec.value(key);
    QVariant expectedValue = expectedValues[key];
    if (actualValue != expectedValue) {
      allMatch = false;
      qDebug() << "Field:" << key << "\n"                                                           //
               << "  Actual: type=" << actualValue.typeName() << ", value=" << actualValue << "\n"  //
               << "  Expected: type=" << expectedValue.typeName() << ", value=" << expectedValue;   //
    }
  }

  int actualRecCount = actualRec.count();
  int actualKeyCount = expectedValues.size();
  if (fullMatch && actualKeyCount != actualRecCount) {
    for (int i = 0; i < actualRecCount; i++) {
      QString fieldName = actualRec.fieldName(i);
      if (!expectedValues.contains(fieldName)) {
        allMatch = false;
        qDebug() << "Unexpected field:" << fieldName << "| Value:" << actualRec.value(i);
      }
    }
  }
  return allMatch;
}

QSqlRecord GetAMovieRecordUsedInBrowser(const QString& prePathLeft, const QString& prePathRight, const QString& name, qint64 sz) {
  using namespace MOVIE_TABLE;
  QSqlRecord rec;
  rec.append(QSqlField(ENUM_2_STR(PrePathLeft), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(PrePathRight), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Name), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Size), QVariant::LongLong));
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::PrePathLeft, prePathLeft);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::PrePathRight, prePathRight);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::Name, name);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELED::Size, sz);
  return rec;
}

}  // namespace SqlRecordTestHelper
