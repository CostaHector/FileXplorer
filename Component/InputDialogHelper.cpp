#include "InputDialogHelper.h"
#include <QFontDialog>
#include <QInputDialog>

namespace InputDialogHelper {

std::pair<bool, QFont> GetFontWithInitial(const QFont& initialFont, QWidget* parent, const QString& title) {
  bool bAccept{false};
  const QFont& newFont = QFontDialog::getFont(&bAccept, initialFont, parent, title);
  return {bAccept, newFont};
}
std::pair<bool, int> GetIntWithInitial(QWidget *parent, const QString &title, const QString &label, int initialValue,
                                       int minValue, int maxValue,
                                       int step) {
  bool bAccept{false};
  int newValue = QInputDialog::getInt(parent, title, label, initialValue, minValue, maxValue, step, &bAccept);
  return {bAccept, newValue};
}

bool YesOrCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon, const QString& title, const QString& text, const QString& informativeText, const QString& detailedText) {
  QMessageBox msgBox;
  msgBox.setIcon(iconType);
  msgBox.setWindowIcon(winIcon);
  msgBox.setWindowTitle(title);
  msgBox.setText(text);
  msgBox.setInformativeText(informativeText);
  msgBox.setDetailedText(detailedText);
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  return msgBox.exec() == QMessageBox::Yes;
}

}
