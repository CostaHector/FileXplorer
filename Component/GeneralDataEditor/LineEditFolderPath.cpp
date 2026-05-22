#include "LineEditFolderPath.h"
#include "NotificatorMacro.h"
#include "SystemPath.h"
#include <QFileDialog>

void LineEditFolderPath::onActionTriggered() {
  const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog | QFileDialog::Option::ShowDirsOnly};
  QString folderPath = QFileDialog::getExistingDirectory(this, "Select an folder", SystemPath::HomePath(), parmOptions);
  if (folderPath.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel select an folder");
    return;
  }
  setText(normalizePath(folderPath));
}
