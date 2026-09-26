#include "SqlRecordTestHelper.h"
#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "MovieDBModelField.h"
#include "FdBasedDb.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include <QSqlField>
#include <QVariant>
#include <QDebug>

namespace SqlRecordTestHelper {

QSqlRecord GetACastRecordLine(const QString& castName, const QString& ori, const QString& imgs, const QString& vids) {
  using namespace CastDbModelField;
  QSqlRecord rec;
  rec.append(QSqlField(ENUM_2_STR(Name), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Rate), QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(ALIAS), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Tags), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Ori), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Height), QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(Size), QVariant::Int));
  rec.append(QSqlField(ENUM_2_STR(Birth), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Vids), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Imgs), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Detail), QVariant::String));

  rec.setValue(Name, castName);
  rec.setValue(Rate, 9);
  rec.setValue(ALIAS, "Thor");
  rec.setValue(Tags, "Man");
  rec.setValue(Ori, ori);
  rec.setValue(Height, -1);
  rec.setValue(Size, -1);
  rec.setValue(Birth, "");
  rec.setValue(Vids, vids);
  rec.setValue(Imgs, imgs);
  rec.setValue(Detail, "The Australian actor first appeared as Thor in 2011 and has since reprised the role in numerous MCU films");
  return rec;
}

bool CheckRecordIfEqual(const QSqlRecord& actualRec,
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) const decltype(defaultValue)& _##enu,
                        PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
                        bool fullMatch) {
  using namespace CastDbModelField;

  const QVariantHash expectedValues {CastPsonFileHelper::PerformerJsonJoiner(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) _##enu,
      PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
      nullptr)};

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
  QSqlRecord rec;
  using namespace MovieDBModelField;
  rec.append(QSqlField(ENUM_2_STR(PrePathLeft), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(PrePathRight), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Name), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Size), QVariant::LongLong));
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELD::PrePathLeft, prePathLeft);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELD::PrePathRight, prePathRight);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Name, name);
  rec.setValue((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Size, sz);
  return rec;
}

QSqlRecord GetAMovieRecordFromDb(const QString& sampleMd5, const QString& prePathLeft, const QString& prePathRight, const QString& name, qint64 sz) {
  QSqlRecord rec;
  using namespace MovieDBModelField;
  rec.append(QSqlField(ENUM_2_STR(SampleMD5), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(PrePathLeft), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(PrePathRight), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Name), QVariant::String));
  rec.append(QSqlField(ENUM_2_STR(Size), QVariant::LongLong));
  rec.append(QSqlField(ENUM_2_STR(Duration), QVariant::Int));     //
  rec.append(QSqlField(ENUM_2_STR(Studio), QVariant::String));       //
  rec.append(QSqlField(ENUM_2_STR(Cast),QVariant::String));         //
  rec.append(QSqlField(ENUM_2_STR(Tags), QVariant::String));        //
  rec.append(QSqlField(ENUM_2_STR(Rate),QVariant::Int));         //
  rec.append(QSqlField(ENUM_2_STR(Detail),QVariant::String));       //
  rec.append(QSqlField(ENUM_2_STR(PathHash),QVariant::String));     //
  rec.append(QSqlField(ENUM_2_STR(InLocal),QVariant::Int));
  const QString fullPath = PathTool::RMFComponent::join(sampleMd5, prePathLeft, prePathRight);
  rec.setValue((int)MovieDBModelField::FIELD_E::SampleMD5, sampleMd5);
  rec.setValue((int)MovieDBModelField::FIELD_E::PrePathLeft, prePathLeft);
  rec.setValue((int)MovieDBModelField::FIELD_E::PrePathRight, prePathRight);
  rec.setValue((int)MovieDBModelField::FIELD_E::Name, name);
  rec.setValue((int)MovieDBModelField::FIELD_E::Size, sz);
  rec.setValue((int)MovieDBModelField::FIELD_E::PathHash, JsonHelper::CalcFileHash(fullPath));
  return rec;
}

}  // namespace SqlRecordTestHelper
