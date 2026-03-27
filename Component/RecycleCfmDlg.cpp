#include "RecycleCfmDlg.h"
#include <QMessageBox>
#include <QIcon>

namespace RecycleCfmDlg {

bool YesOrCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon, const QString& title, const QString& text, const QString& InformativeText) {
  QMessageBox msgBox;
  msgBox.setIcon(iconType);
  msgBox.setWindowIcon(winIcon);
  msgBox.setWindowTitle(title);
  msgBox.setText(text);
  msgBox.setInformativeText(InformativeText);
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  return msgBox.exec() == QMessageBox::Yes;
}

bool recycleQuestion(const QString& locatedIn, const QStringList& files, bool bDeletePermanently) {
  QString recycleMsg;
  recycleMsg.reserve(80);
  if (bDeletePermanently) {
    recycleMsg += "<b>WARNING: This action cannot be undone!</b> ";
  }
  recycleMsg += QString{"Following <b>%1</b> item(s) under path[%2] "}.arg(files.size()).arg(locatedIn);
  recycleMsg += bDeletePermanently ? "will be permanently deleted." : "will be moved to the Recycle Bin.";

  QString recycleDetailMsg;
  static constexpr int MAX_FILE_NAME_DISP = 20;
  if (files.size() > MAX_FILE_NAME_DISP) {
    recycleDetailMsg += files.mid(0, MAX_FILE_NAME_DISP).join('\n');
    recycleDetailMsg += QString("\n---\n... and %1 more item(s)\n---\n").arg(files.size() - MAX_FILE_NAME_DISP);
    recycleDetailMsg += files.constLast();
  } else {
    recycleDetailMsg += files.join('\n');
  }
  return YesOrCancelBox(bDeletePermanently ? QMessageBox::Icon::Warning : QMessageBox::Icon::Question,                  //
                        bDeletePermanently ? QIcon(":img/DELETE_ITEMS_PERMANENTLY") : QIcon(":img/MOVE_TO_TRASH_BIN"),  //
                        bDeletePermanently ? "Permanently Delete Files" : "Move to Recycle Bin",                        //
                        recycleMsg,                                                                                     //
                        recycleDetailMsg);
}

}  // namespace RecycleCfmDlg
