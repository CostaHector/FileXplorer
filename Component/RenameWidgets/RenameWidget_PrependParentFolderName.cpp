#include "RenameWidget_PrependParentFolderName.h"
#include "PublicMacro.h"
#include "RenameHelper.h"
#include "SpacerWidget.h"

RenameWidget_PrependParentFolderName::RenameWidget_PrependParentFolderName(QWidget* parent)  //
  : AdvanceRenamer{parent} {
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
  auto* pSpacer = GetSpacerWidget(prependTb, Qt::Orientation::Horizontal);
  prependTb->addWidget(pSpacer);
  prependTb->addWidget(m_recursiveCB);
  return prependTb;
}

void RenameWidget_PrependParentFolderName::extraSubscribe() {}

void RenameWidget_PrependParentFolderName::InitExtraMemberWidget() {}

QStringList RenameWidget_PrependParentFolderName::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty() || mRelToNameWithNoRoot.isEmpty()) {
    return replaceeList;
  }
  const QStringList& suffixs = m_oExtTE->toPlainText().split(NAME_SEP);
  return RenameHelper::PrependParentFolderNameToFileName(mRelToNameWithNoRoot, replaceeList, suffixs);
}
