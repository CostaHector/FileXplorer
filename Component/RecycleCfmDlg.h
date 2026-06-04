#ifndef RECYCLECFMDLG_H
#define RECYCLECFMDLG_H

#include <QStringList>

namespace RecycleCfmDlg {
bool recycleQuestion(const QString& locatedIn, const QStringList& files, bool bDeletePermanently = false);
}

#endif  // RECYCLECFMDLG_H
