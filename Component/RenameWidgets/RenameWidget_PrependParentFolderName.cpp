#include "RenameWidget_PrependParentFolderName.h"
#include "PublicMacro.h"
#include "RenameHelper.h"
#include "SpacerWidget.h"

void RenameWidget_PrependParentFolderName::initExclusiveSetting() {
  m_recursiveCB->setEnabled(true);
  m_recursiveCB->setChecked(true);
}

void RenameWidget_PrependParentFolderName::InitExtraCommonVariable() {
  windowTitleFormat = "Prepend parend folder names | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/PREPEND_PARENT_FOLDER_NAMES"));
}

QToolBar* RenameWidget_PrependParentFolderName::InitControlTB() {
  QToolBar* prependTb{new (std::nothrow) QToolBar{"PrependParentFolderNamesTB", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(prependTb);
  auto* pSpacer = SpacerWidget::GetSpacerWidget(prependTb, Qt::Orientation::Horizontal);
  prependTb->addWidget(pSpacer);
  prependTb->addWidget(m_nameExtIndependent);
  prependTb->addWidget(m_recursiveCB);
  prependTb->addWidget(regexValidLabel);
  return prependTb;
}

QStringList RenameWidget_PrependParentFolderName::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty() || mRelToNameWithNoRoot.isEmpty()) {
    return replaceeList;
  }
  return RenameHelper::PrependParentFolderNameToFileName(mRelToNameWithNoRoot, replaceeList, mExts);
}
