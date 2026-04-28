#include "LineEditFolderPath.h"
#include "NotificatorMacro.h"
#include "PublicVariable.h"
#include <QFileDialog>

void LineEditFolderPath::onActionTriggered() {
  const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog | QFileDialog::Option::ShowDirsOnly};
  QString folderPath = QFileDialog::getExistingDirectory(this, "Select an folder", SystemPath::HOME_PATH(), parmOptions);
  if (folderPath.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel select an folder");
    return;
  }
  setText(normalizePath(folderPath));
}
