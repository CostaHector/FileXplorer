#ifndef SQLRECORDTESTHELPER_H
#define SQLRECORDTESTHELPER_H

#include <QSqlRecord>
#include <QString>
namespace SqlRecordTestHelper {

QSqlRecord GetACastRecordLine(const QString& castName,
                              const QString& ori,
                              const QString& imgs,
                              const QString& vids = "The Avengers (2012)\nThor: The Dark World (2013)\nAvengers: Age of Ultron (2015)");
bool CheckRecordIfEqual(const QSqlRecord& expectRec,
                        const QString& name,
                        const int rate,
                        const QString& aka,
                        const QString& tags,
                        const QString& ori,
                        const QString& vids,
                        const QString& imgs,
                        const QString& detail,
                        bool fullMatch = false);
QSqlRecord GetAMovieRecordUsedInBrowser(const QString& prePathLeft, const QString& prePathRight, const QString& name, qint64 sz);

}  // namespace SqlRecordTestHelper

#endif  // SQLRECORDTESTHELPER_H
