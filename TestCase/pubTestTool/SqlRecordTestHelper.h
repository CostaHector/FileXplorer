#ifndef SQLRECORDTESTHELPER_H
#define SQLRECORDTESTHELPER_H

#include <QSqlRecord>
#include <QString>
#include "CastPsonFileHelper.h"

namespace SqlRecordTestHelper {

QSqlRecord GetACastRecordLine(const QString& castName,
                              const QString& ori,
                              const QString& imgs,
                              const QString& vids = "The Avengers (2012)\nThor: The Dark World (2013)\nAvengers: Age of Ultron (2015)");

bool CheckRecordIfEqual(const QSqlRecord& expectRec,
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) const decltype(defaultValue)& _##enu,
                        PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
                        bool fullMatch = false);
QSqlRecord GetAMovieRecordUsedInBrowser(const QString& prePathLeft, const QString& prePathRight, const QString& name, qint64 sz);

}  // namespace SqlRecordTestHelper

#endif  // SQLRECORDTESTHELPER_H
