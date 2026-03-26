#ifndef RECYCLECFMDLG_H
#define RECYCLECFMDLG_H

#include <QStringList>
#include <QMessageBox>

namespace RecycleCfmDlg {
bool YesOrCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon, const QString& title, const QString& text, const QString& InformativeText);
bool recycleQuestion(const QString& locatedIn, const QStringList& files, bool bDeletePermanently = false);
}

#endif  // RECYCLECFMDLG_H
