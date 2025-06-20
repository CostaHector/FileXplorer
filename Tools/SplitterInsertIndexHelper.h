#ifndef SPLITTERINSERTINDEXHELPER_H
#define SPLITTERINSERTINDEXHELPER_H

#include <QVector>
void MoveElementFrontOf(QVector<int>& v, int fromIndex, int destIndex);

bool IsValidMediaTypeSeq(const QString& seqStr, QVector<int>& result);
QString MediaTypeSeqStr(const QVector<int>& result);
#endif  // SPLITTERINSERTINDEXHELPER_H
