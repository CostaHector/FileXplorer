#ifndef RECYCLECFMDLG_H
#define RECYCLECFMDLG_H

#include <QStringList>

namespace RecycleCfmDlg {
bool recycleQuestion(const QString& locatedIn, const QStringList& files, bool bDeletePermanently = false);
bool archiveQuestion(const QString& locatedIn, const QStringList& files, const QString& destinationIn);
}

#endif  // RECYCLECFMDLG_H
